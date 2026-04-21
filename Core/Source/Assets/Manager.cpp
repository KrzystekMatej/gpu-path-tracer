#include <Core/Assets/Manager.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include <Core/Import/ObjLoader.hpp>
#include <Core/Import/ImageLoader.hpp>
#include <Core/Import/ShaderLoader.hpp>
#include <Core/Graphics/Gl/Material.hpp>

namespace Core::Assets
{
	namespace
	{
		constexpr std::string_view DefaultMaterialSource = "default_material";
		constexpr std::string_view BackgroundFile = "background.exr";
		constexpr std::string_view IrradianceDir = "irradiance_map";
		constexpr std::string_view PrefilteredDir = "prefiltered_map";
		constexpr std::string_view SkyboxDir = "skybox";


		std::filesystem::path ResolveRelativeToFile(const std::filesystem::path& filePath, const std::filesystem::path& maybeRelative)
		{
			auto resolved = Utils::Path::ResolvePath(maybeRelative, filePath.parent_path());

			if (!resolved)
				return resolved.value().absolute;

			return maybeRelative.lexically_normal();
		}

		uint8_t NormalizedFloatToU8(float x)
		{
			if (!std::isfinite(x))
				x = 0.0f;
			x = std::clamp(x, 0.0f, 1.0f);
			return static_cast<uint8_t>(std::lround(x * 255.0f));
		}

		bool IsUint8Texture(const Graphics::PixelFormat& f)
		{
			return f.componentType == Graphics::ComponentType::UInt8;
		}

		bool IsHdrTexture(const Graphics::PixelFormat& f)
		{
			return f.componentType == Graphics::ComponentType::Float16 || f.componentType == Graphics::ComponentType::Float32;
		}

		std::expected<Core::Graphics::Gl::Texture, Utils::Error> LoadCubemapTexture(const std::filesystem::path& folder, Graphics::ColorSpace colorSpace)
		{
			auto mipChain = Import::LoadCubemapMipChainFromFolder(folder, colorSpace);
			if (mipChain)
				return Core::Graphics::Gl::Texture::CreateCubemapFromMipmaps(mipChain.value());
			auto cubemap = Import::LoadCubemapFromFolder(folder, colorSpace, "");
			if (!cubemap)
				return std::unexpected(cubemap.error());

			return Core::Graphics::Gl::Texture::CreateCubemap(cubemap.value());
		}
	}

	std::expected<Manager, Utils::Error> Manager::Create(std::filesystem::path assetsPath)
	{
		 std::error_code errorCode;
		assetsPath = std::filesystem::weakly_canonical(std::filesystem::absolute(assetsPath), errorCode);
		if (errorCode) return std::unexpected(Utils::Error("Invalid assets root: {}", assetsPath.string()));
		if (!std::filesystem::is_directory(assetsPath))
			return std::unexpected(Utils::Error("Asset root directory '{}' does not exist!", assetsPath.string()));

		Manager manager;
		manager.m_Root = std::move(assetsPath);
		auto defaultMaterial = manager.ImportDefaultMaterial();
		if (!defaultMaterial)
			return std::unexpected(Utils::Error(std::make_shared<Utils::Error>(defaultMaterial.error()), "Failed to import default material"));
		return manager;
	}

	std::expected<Handle<Texture>, Utils::Error> Manager::ImportPixelTexture(const Graphics::PixelFormat& format, std::span<const uint8_t> data)
	{
		Source source = SourcePixel{ format, data };
		Subkey subkey = SubkeyNone{};

		auto cached = m_Storage.GetHandleByKey<Texture>(source, subkey);
		if (cached)
			return cached.value();

		Import::Image image{
			.width = 1,
			.height = 1,
			.format = format,
			.data = std::vector<uint8_t>(data.begin(), data.end())
		};

		auto glResult = Graphics::Gl::Texture::Create2D(image);
		if (!glResult)
			return std::unexpected(Utils::Error(std::move(glResult).error()));

		auto cudaResult = Graphics::Cuda::Texture::Create2D(image);
		if (!cudaResult)
			return std::unexpected(Utils::Error(std::move(cudaResult).error()));

		Texture asset(
			Graphics::Cpu::Texture::Create(std::move(image)),
			std::move(glResult).value(),
			std::move(cudaResult).value());

		auto handle = m_Storage.Emplace(source, subkey, std::move(asset));
		return handle.value();
	}

	std::expected<Handle<Material>, Utils::Error> Manager::ImportDefaultMaterial()
	{
		Source source = SourcePath{ DefaultMaterialSource };
		Subkey subkey = SubkeyNone{};

		auto cached = m_Storage.GetHandleByKey<Material>(source, subkey);
		if (cached)
			return cached.value();

		Graphics::PixelFormat rgbSrgb
		{
			.layout = Graphics::ChannelLayout::RGB,
			.componentType = Graphics::ComponentType::UInt8,
			.colorSpace = Graphics::ColorSpace::SRGB
		};

		Graphics::PixelFormat rLinear
		{
			.layout = Graphics::ChannelLayout::R,
			.componentType = Graphics::ComponentType::UInt8,
			.colorSpace = Graphics::ColorSpace::Linear
		};

		Graphics::PixelFormat rgbLinear
		{
			.layout = Graphics::ChannelLayout::RGB,
			.componentType = Graphics::ComponentType::UInt8,
			.colorSpace = Graphics::ColorSpace::Linear
		};

		auto albedo = ImportPixelTexture(rgbSrgb, Graphics::MaterialDefaults::DefaultAlbedo);
		if (!albedo)
			return std::unexpected(albedo.error());

		auto roughness = ImportPixelTexture(rLinear, Graphics::MaterialDefaults::DefaultRoughness);
		if (!roughness)
			return std::unexpected(roughness.error());

		auto metallic = ImportPixelTexture(rLinear, Graphics::MaterialDefaults::DefaultMetallic);
		if (!metallic)
			return std::unexpected(metallic.error());

		auto ao = ImportPixelTexture(rLinear, Graphics::MaterialDefaults::DefaultAo);
		if (!ao)
			return std::unexpected(ao.error());

		auto normal = ImportPixelTexture(rgbLinear, Graphics::MaterialDefaults::DefaultNormal);
		if (!normal)
			return std::unexpected(normal.error());

		Material asset(
			Graphics::MaterialDefaults::DefaultSurfaceModel,
			albedo.value(),
			roughness.value(),
			metallic.value(),
			ao.value(),
			normal.value());

		auto handle = m_Storage.Emplace(source, subkey, std::move(asset));
		return handle.value();
	}

	std::expected<Handle<Texture>, Utils::Error> Manager::ImportTexture(
		const std::filesystem::path& path, 
		Graphics::ColorSpace colorSpace, 
		std::optional<std::filesystem::path> root)
	{
		std::filesystem::path actualRoot = root ? *root : m_Root;
		auto pathResult = Utils::Path::ResolvePath(path, actualRoot);
		if (!pathResult)
			return std::unexpected(std::move(pathResult).error());
		Utils::Path::ResolvedPath resolvedPath = std::move(pathResult).value();
		std::string absoluteStr = resolvedPath.absolute.generic_string();

		std::string relativeStr = resolvedPath.relative.generic_string();
		if (root)
			relativeStr = std::filesystem::relative(resolvedPath.absolute, m_Root).generic_string();

		Source source = SourcePath{ relativeStr };
		Subkey subkey = SubkeyNone{};

		auto cached = m_Storage.GetHandleByKey<Texture>(source, subkey);
		if (cached)
			return cached.value();

		auto imageResult = Import::LoadImage(resolvedPath.absolute, colorSpace);
		if (!imageResult)
			return std::unexpected(Utils::Error(
				std::make_shared<Utils::Error>(imageResult.error()), 
				"Failed to load image, file path: {}", 
				absoluteStr));

		Import::Image image = std::move(imageResult.value());

		auto glResult = Graphics::Gl::Texture::Create2D(image);
		if (!glResult)
			return std::unexpected(Utils::Error(
				std::make_shared<Utils::Error>(glResult.error()), 
				"Failed to create GL texture, file path: {}", 
				absoluteStr));

		auto cudaResult = Graphics::Cuda::Texture::Create2D(image);
		if (!cudaResult)
			return std::unexpected(Utils::Error(
				std::make_shared<Utils::Error>(cudaResult.error()), 
				"Failed to create GL texture, file path: {}", 
				absoluteStr));

		Texture asset(
			Graphics::Cpu::Texture::Create(std::move(image)),
			std::move(glResult).value(),
			std::move(cudaResult).value());

		auto handle = m_Storage.Emplace(source, subkey, std::move(asset));
		return handle.value();
	}

	std::expected<Handle<Mesh>, Utils::Error> Manager::ImportMesh(const Utils::Path::ResolvedPath& objPath, Import::ParsedMesh mesh, uint32_t index)
	{
		std::string relativeStr = objPath.relative.generic_string();
		Source source = SourcePath{ relativeStr };
		Subkey subkey = SubkeyIndex{ index };

		auto cached = m_Storage.GetHandleByKey<Mesh>(source, subkey);
		if (cached)
			return cached.value();

		auto glResult = Graphics::Gl::Mesh::Create(mesh);
		if (!glResult)
			return std::unexpected(Utils::Error(
				std::make_shared<Utils::Error>(glResult.error()),
				"Failed to create GL mesh, obj path: {}",
				objPath.absolute.generic_string()));

		Mesh asset(Graphics::Cpu::Mesh::Create(std::move(mesh)), std::move(glResult.value()));

		auto handle = m_Storage.Emplace(source, subkey, std::move(asset));
		return handle.value();
	}

	std::expected<Handle<Material>, Utils::Error> Manager::ImportMaterial(const Utils::Path::ResolvedPath& objPath, const Import::ParsedMaterial& material)
	{
		std::string relativeStr = objPath.relative.generic_string();
		Source source = SourcePath{ relativeStr };
		Subkey subkey = SubkeyName{ material.name };

		auto cached = m_Storage.GetHandleByKey<Material>(source, subkey);
		if (cached)
			return cached.value();

		const auto makeRgb = [&](Graphics::ColorSpace cs, std::array<uint8_t, 3> v)
		{
			Graphics::PixelFormat f{
				.layout = Graphics::ChannelLayout::RGB,
				.componentType = Graphics::ComponentType::UInt8,
				.colorSpace = cs
			};
			return ImportPixelTexture(f, v);
		};

		const auto makeR = [&](std::array<uint8_t, 1> v)
		{
			Graphics::PixelFormat f{
				.layout = Graphics::ChannelLayout::R,
				.componentType = Graphics::ComponentType::UInt8,
				.colorSpace = Graphics::ColorSpace::Linear
			};
			return ImportPixelTexture(f, v);
		};

		std::array<uint8_t, 3> albedoPixel = 
		{
			NormalizedFloatToU8(material.albedo[0]),
			NormalizedFloatToU8(material.albedo[1]),
			NormalizedFloatToU8(material.albedo[2])
		};
		std::array<uint8_t, 1> roughnessPixel = { NormalizedFloatToU8(material.roughness) };
		std::array<uint8_t, 1> metallicPixel = { NormalizedFloatToU8(material.metallic)};

		std::optional<std::filesystem::path> root = objPath.absolute.parent_path();

		auto albedo = material.albedoTexture
			? ImportTexture(material.albedoTexture.value(), Graphics::ColorSpace::SRGB, root)
			: makeRgb(Graphics::ColorSpace::SRGB, albedoPixel);

		if (!albedo) return std::unexpected(albedo.error());

		auto roughness = material.roughnessTexture
			? ImportTexture(material.roughnessTexture.value(), Graphics::ColorSpace::Linear, root)
			: makeR(roughnessPixel);

		if (!roughness) return std::unexpected(roughness.error());

		auto metallic = material.metallicTexture
			? ImportTexture(material.metallicTexture.value(), Graphics::ColorSpace::Linear, root)
			: makeR(metallicPixel);

		if (!metallic) return std::unexpected(metallic.error());

		auto ao = material.aoTexture
			? ImportTexture(material.aoTexture.value(), Graphics::ColorSpace::Linear, root)
			: makeR(Graphics::MaterialDefaults::DefaultAo);

		if (!ao) return std::unexpected(ao.error());

		auto normal = material.normalTexture
			? ImportTexture(material.normalTexture.value(), Graphics::ColorSpace::Linear, root)
			: makeRgb(Graphics::ColorSpace::Linear, Graphics::MaterialDefaults::DefaultNormal);

		if (!normal) return std::unexpected(normal.error());

		auto localShading = Graphics::Gl::ToLocalShadingChecked(material.surface);
		if (!localShading.second)
			spdlog::warn(
				"Material '{}' uses surface model, which is not supported with local illumination."
				"It will be treated as Unlit when shading via OpenGL. File path: {}",
				material.name,
				objPath.absolute.generic_string());

		Material asset(
			material.surface,
			albedo.value(),
			roughness.value(),
			metallic.value(),
			ao.value(),
			normal.value());

		auto handle = m_Storage.Emplace(source, subkey, std::move(asset));
		return handle.value();
	}

	std::expected<Handle<Model>, Utils::Error> Manager::ImportObj(const std::filesystem::path& path)
	{
		auto pathResult = Utils::Path::ResolvePath(path, m_Root);
		if (!pathResult)
			return std::unexpected(pathResult.error());
		Utils::Path::ResolvedPath resolved = pathResult.value();

		std::string absoluteStr = resolved.absolute.generic_string();
		std::string relativeStr = resolved.relative.generic_string();

		Source source = SourcePath{ relativeStr };
		Subkey subkey = SubkeyNone{};

		auto cached = m_Storage.GetHandleByKey<Model>(source, subkey);
		if (cached)
			return cached.value();

		auto parsedResult = Import::LoadObj(resolved.absolute);
		if (!parsedResult)
			return std::unexpected(Utils::Error(std::move(parsedResult).error()));

		Import::ParsedModel parsed = std::move(parsedResult.value());
		auto defaultMat = ImportDefaultMaterial();
		if (!defaultMat)
			return std::unexpected(defaultMat.error());

		Model model{};
		model.parts.reserve(parsed.meshes.size());

		for (auto& mesh : parsed.meshes)
		{
			Handle<Material> materialHandle = defaultMat.value();

			if (mesh.materialIndex)
			{
				if (*mesh.materialIndex >= parsed.materials.size())
					return std::unexpected(Utils::Error("Invalid material index {}, file path: {}", *mesh.materialIndex, absoluteStr));

				auto handleResult = ImportMaterial(resolved, parsed.materials[*mesh.materialIndex]);
				if (!handleResult)
					return std::unexpected(handleResult.error());

				materialHandle = handleResult.value();
			}

			auto meshHandle = ImportMesh(resolved, std::move(mesh), mesh.index);
			if (!meshHandle)
				return std::unexpected(meshHandle.error());

			model.parts.push_back(ModelPart{
				.mesh = meshHandle.value(),
				.material = materialHandle
			});
		}

		auto handle = m_Storage.Emplace(source, subkey, std::move(model));
		return handle.value();
	}

	std::expected<Handle<EnvironmentMap>, Utils::Error> Manager::ImportEnvironmentMap(const std::filesystem::path& path, Graphics::ColorSpace colorSpace)
	{
		auto pathResult = Utils::Path::ResolvePath(path, m_Root);
		if (!pathResult)
			return std::unexpected(std::move(pathResult).error());
		Utils::Path::ResolvedPath resolved = std::move(pathResult).value();

		std::string absoluteStr = resolved.absolute.generic_string();
		std::string relativeStr = resolved.relative.generic_string();

		Source source = SourcePath{ relativeStr };
		Subkey subkey = SubkeyNone{};

		auto cached = m_Storage.GetHandleByKey<EnvironmentMap>(source, subkey);
		if (cached)
			return cached.value();

		std::filesystem::path backgroundPath = resolved.absolute / std::string(BackgroundFile);
		std::filesystem::path irradiancePath = resolved.absolute / std::string(IrradianceDir);
		std::filesystem::path prefilteredPath = resolved.absolute / std::string(PrefilteredDir);
		std::filesystem::path skyboxPath = resolved.absolute / std::string(SkyboxDir);

		auto backgroundResult = Import::LoadImage(backgroundPath, colorSpace);
		if (!backgroundResult)
			return std::unexpected(Utils::Error(
				std::make_shared<Utils::Error>(backgroundResult.error()), 
				"Failed to load environment background, file path: {}", 
				backgroundPath.string()));

		Import::Image backgroundImage = std::move(backgroundResult.value());

		if (!IsHdrTexture(backgroundImage.format))
			return std::unexpected(Utils::Error(
				"Environment background must be HDR (Float16/Float32), file path: {}", 
				backgroundPath.string()));

		auto skybox = LoadCubemapTexture(skyboxPath, colorSpace);
		if (!skybox)
			return std::unexpected(Utils::Error(
				std::make_shared<Utils::Error>(skybox.error()), 
				"Failed to load skybox cubemap, dir: {}", 
				skyboxPath.string()));

		auto irradiance = LoadCubemapTexture(irradiancePath, colorSpace);
		if (!irradiance)
			return std::unexpected(Utils::Error(std::make_shared<Utils::Error>(
				irradiance.error()), 
				"Failed to load irradiance cubemap, dir: {}", 
				irradiancePath.string()));

		auto prefiltered = LoadCubemapTexture(prefilteredPath, colorSpace);
		if (!prefiltered)
			return std::unexpected(Utils::Error(std::make_shared<Utils::Error>(
				prefiltered.error()), 
				"Failed to load prefiltered cubemap, dir: {}", 
				prefilteredPath.string()));

		EnvironmentMap asset(
			Graphics::Cpu::EnvironmentMap::Create(Graphics::Cpu::Texture::Create(std::move(backgroundImage))),
			Graphics::Gl::EnvironmentMap(std::move(skybox.value()), std::move(irradiance.value()), std::move(prefiltered.value())),
			Graphics::Cuda::EnvironmentMap{}
		);

		auto handle = m_Storage.Emplace(source, subkey, std::move(asset));
		return handle.value();
	}

	std::expected<Handle<ShaderProgram>, Utils::Error> Manager::ImportShaderProgram(
		std::span<std::pair<std::filesystem::path, Graphics::Gl::ShaderType>> shaderPaths)
	{
		
		std::ranges::sort(shaderPaths, [](const auto& a, const auto& b)
		{
			return a.second < b.second;
		});
		
		std::vector<std::pair<Utils::Path::ResolvedPath, Graphics::Gl::ShaderType>> resolvedShaderPaths;
		resolvedShaderPaths.reserve(shaderPaths.size());
		std::string combinedPaths;
		
		for (const auto& [path, shaderType] : shaderPaths)
		{
			auto pathResult = Utils::Path::ResolvePath(path, m_Root);
			if (!pathResult)
				return std::unexpected(std::move(pathResult).error());
			auto resolved = std::move(pathResult).value();
			combinedPaths += resolved.relative.generic_string() + ";";
			resolvedShaderPaths.emplace_back(std::move(resolved), shaderType);
		}
		
		Source source = SourcePath{ combinedPaths };
		Subkey subkey = SubkeyNone{};
		auto cached = m_Storage.GetHandleByKey<ShaderProgram>(source, subkey);
		if (cached)
			return cached.value();

		std::vector<Graphics::Gl::Shader> shaders;
		shaders.reserve(shaderPaths.size());
		for (const auto& [path, shaderType] : resolvedShaderPaths)
		{
			auto loadedShader = Import::LoadShader(path.absolute, shaderType);
			if (!loadedShader)
				return std::unexpected(Utils::Error(std::move(loadedShader).error()));

			auto shader = Graphics::Gl::Shader::Create(std::move(loadedShader).value());
			if (!shader)
				return std::unexpected(Utils::Error(std::move(shader).error()));
			shaders.emplace_back(std::move(shader).value());
		}

		auto program = Graphics::Gl::ShaderProgram::Create(shaders);
		if (!program)
			return std::unexpected(Utils::Error(std::move(program).error()));

		ShaderProgram programAsset(std::move(program).value());
		return m_Storage.Emplace(source, subkey, std::move(programAsset));
	}
}
