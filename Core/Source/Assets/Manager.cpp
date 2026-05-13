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
#include <Core/Graphics/Cuda/PathTracing/Material.hpp>

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

		float NormalizeFloat(float x, float min, float max)
		{
			if (!std::isfinite(x))
				x = min;
			x = std::clamp(x, min, max);
			return (x - min) / (max - min);
		}

		uint8_t NormalizedFloatToU8(float x)
		{
			if (!std::isfinite(x))
				x = 0.0f;
			x = std::clamp(x, 0.0f, 1.0f);
			return static_cast<uint8_t>(std::lround(x * 255.0f));
		}
		
		std::array<uint8_t, 3> RgbToU8(const float color[3])
		{
			return {
				NormalizedFloatToU8(color[0]),
				NormalizedFloatToU8(color[1]),
				NormalizedFloatToU8(color[2])
			};
		}
		
		uint8_t LinearNormalizedFloatToSrgbU8(float x)
		{
			if (!std::isfinite(x))
				x = 0.0f;

			x = std::clamp(x, 0.0f, 1.0f);

			float srgb;
			if (x <= 0.0031308f)
				srgb = 12.92f * x;
			else
				srgb = 1.055f * std::pow(x, 1.0f / 2.4f) - 0.055f;

			return static_cast<uint8_t>(std::lround(srgb * 255.0f));
		}
		
		std::array<uint8_t, 3> LinearRgbToSrgbU8(const float color[3])
		{
			return {
				LinearNormalizedFloatToSrgbU8(color[0]),
				LinearNormalizedFloatToSrgbU8(color[1]),
				LinearNormalizedFloatToSrgbU8(color[2])
			};
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
		CORE_TRY_DISCARD_CONTEXT(manager.ImportDefaultMaterial(), "Failed to import default material");
		return manager;
	}

	std::expected<Handle<Texture>, Utils::Error> Manager::ImportPixelTexture(const Graphics::PixelFormat& format, std::span<const uint8_t> value)
	{
		Source source = SourcePixel{ format, value };
		Subkey subkey = SubkeyNone{};

		auto cached = m_Storage.GetHandleByKey<Texture>(source, subkey);
		if (cached)
			return cached.value();

		Import::Image image{
			.width = 1,
			.height = 1,
			.format = format,
			.data = std::vector<uint8_t>(value.begin(), value.end())
		};

		CORE_TRY(glTexture, Graphics::Gl::Texture::Create2D(image));
		CORE_TRY(cudaTexture, Graphics::Cuda::Texture::Create2D(image));

		Texture asset(
			Graphics::Cpu::Texture::Create(std::move(image)),
			std::move(glTexture),
			std::move(cudaTexture));

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

		Graphics::PixelFormat rgbLinearHdr
		{
			.layout = Graphics::ChannelLayout::RGB,
			.componentType = Graphics::ComponentType::Float32,
			.colorSpace = Graphics::ColorSpace::Linear
		};

		CORE_TRY_CONTEXT(albedo, ImportPixelTexture(rgbSrgb, Graphics::MaterialDefaults::DefaultAlbedo), "Failed to import albedo texture");
		CORE_TRY_CONTEXT(specular, ImportPixelTexture(rgbSrgb, Graphics::MaterialDefaults::DefaultPhongSpecular), "Failed to import specular texture");
		CORE_TRY_CONTEXT(shininess, ImportPixelTexture(rLinear, Graphics::MaterialDefaults::DefaultShininess), "Failed to import shininess texture");
		CORE_TRY_CONTEXT(roughness, ImportPixelTexture(rLinear, Graphics::MaterialDefaults::DefaultRoughness), "Failed to import roughness texture");
		CORE_TRY_CONTEXT(metallic, ImportPixelTexture(rLinear, Graphics::MaterialDefaults::DefaultMetallic), "Failed to import metallic texture");
		CORE_TRY_CONTEXT(ao, ImportPixelTexture(rLinear, Graphics::MaterialDefaults::DefaultAo), "Failed to import ambient occlusion texture");
		
		const std::array<float, 3>& emissionDefault = Graphics::MaterialDefaults::DefaultEmission;
		CORE_TRY_CONTEXT(emission, ImportPixelTexture(rgbLinearHdr, { reinterpret_cast<const uint8_t*>(&emissionDefault[0]), sizeof(emissionDefault) }), "Failed to import emission texture");
		CORE_TRY_CONTEXT(normal, ImportPixelTexture(rgbLinear, Graphics::MaterialDefaults::DefaultNormal), "Failed to import normal texture");

		Material asset(
			Graphics::MaterialDefaults::DefaultSurfaceModel,
			albedo,
			specular,
			shininess,
			roughness,
			metallic,
			ao,
			emission,
			normal);

		auto handle = m_Storage.Emplace(source, subkey, std::move(asset));
		return handle.value();
	}

	std::expected<Handle<Texture>, Utils::Error> Manager::ImportTexture(
		const std::filesystem::path& path, 
		Graphics::ColorSpace colorSpace, 
		std::optional<std::filesystem::path> root)
	{
		std::filesystem::path actualRoot = root ? *root : m_Root;
		CORE_TRY(resolvedPath, Utils::Path::ResolvePath(path, actualRoot));
		
		std::string absoluteStr = resolvedPath.absolute.generic_string();
		std::string relativeStr = resolvedPath.relative.generic_string();
		if (root)
			relativeStr = std::filesystem::relative(resolvedPath.absolute, m_Root).generic_string();

		Source source = SourcePath{ relativeStr };
		Subkey subkey = SubkeyNone{};

		auto cached = m_Storage.GetHandleByKey<Texture>(source, subkey);
		if (cached)
			return cached.value();

		CORE_TRY(image, Import::LoadImage(resolvedPath.absolute, colorSpace));

		CORE_TRY_CONTEXT(glTexture, Graphics::Gl::Texture::Create2D(image), "Failed to create GL texture, file path: {}", absoluteStr);
		CORE_TRY_CONTEXT(cudaTexture, Graphics::Cuda::Texture::Create2D(image), "Failed to create CUDA texture, file path: {}", absoluteStr);
		Graphics::Cpu::Texture cpuTexture = Graphics::Cpu::Texture::Create(std::move(image));

		Texture asset(
			std::move(cpuTexture),
			std::move(glTexture),
			std::move(cudaTexture));

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

		CORE_TRY_CONTEXT(glMesh, Graphics::Gl::Mesh::Create(mesh), "Failed to create GL mesh, obj path: {}", objPath.absolute.generic_string());
		Graphics::Cpu::Mesh cpuMesh = Graphics::Cpu::Mesh::Create(std::move(mesh));

		Mesh asset(std::move(cpuMesh), std::move(glMesh));
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

		const auto makePixel = [&](Graphics::ChannelLayout layout, Graphics::ComponentType componentType, Graphics::ColorSpace colorSpace, std::span<const uint8_t> value)
		{
			Graphics::PixelFormat format{
				.layout = layout,
				.componentType = componentType,
				.colorSpace = colorSpace
			};
			return ImportPixelTexture(format, value);
		};

		std::array<uint8_t, 3> albedoPixel = LinearRgbToSrgbU8(material.albedo);
		std::array<uint8_t, 3> specularPixel = LinearRgbToSrgbU8(material.specular);
		std::array<uint8_t, 1> shininessPixel = { NormalizedFloatToU8(NormalizeFloat(material.shininess, Graphics::MaterialDefaults::MinShininess, Graphics::MaterialDefaults::MaxShininess)) };
		std::array<uint8_t, 1> roughnessPixel = { NormalizedFloatToU8(material.roughness) };
		std::array<uint8_t, 1> metallicPixel = { NormalizedFloatToU8(material.metallic) };
		std::span<const uint8_t> emissionPixel = {
			reinterpret_cast<const uint8_t*>(&material.emission[0]),
			sizeof(material.emission)
		};

		std::optional<std::filesystem::path> root = objPath.absolute.parent_path();

		auto albedo = material.albedoTexture
			? ImportTexture(material.albedoTexture.value(), Graphics::ColorSpace::SRGB, root)
			: makePixel(Graphics::ChannelLayout::RGB, Graphics::ComponentType::UInt8, Graphics::ColorSpace::SRGB, albedoPixel);

		if (!albedo) return std::unexpected(std::move(albedo).error());

		auto specular = material.specularTexture
			? ImportTexture(material.specularTexture.value(), Graphics::ColorSpace::SRGB, root)
			: makePixel(Graphics::ChannelLayout::RGB, Graphics::ComponentType::UInt8, Graphics::ColorSpace::SRGB, specularPixel);
		
		if (!specular) return std::unexpected(std::move(specular).error());

		auto shininess = material.shininessTexture
			? ImportTexture(material.shininessTexture.value(), Graphics::ColorSpace::Linear, root)
			: makePixel(Graphics::ChannelLayout::R, Graphics::ComponentType::UInt8, Graphics::ColorSpace::Linear, shininessPixel);

		if (!shininess) return std::unexpected(std::move(shininess).error());

		auto roughness = material.roughnessTexture
			? ImportTexture(material.roughnessTexture.value(), Graphics::ColorSpace::Linear, root)
			: makePixel(Graphics::ChannelLayout::R, Graphics::ComponentType::UInt8, Graphics::ColorSpace::Linear, roughnessPixel);

		if (!roughness) return std::unexpected(std::move(roughness).error());

		auto metallic = material.metallicTexture
			? ImportTexture(material.metallicTexture.value(), Graphics::ColorSpace::Linear, root)
			: makePixel(Graphics::ChannelLayout::R, Graphics::ComponentType::UInt8, Graphics::ColorSpace::Linear, metallicPixel);

		if (!metallic) return std::unexpected(std::move(metallic).error());

		auto ao = material.aoTexture
			? ImportTexture(material.aoTexture.value(), Graphics::ColorSpace::Linear, root)
			: makePixel(Graphics::ChannelLayout::R, Graphics::ComponentType::UInt8, Graphics::ColorSpace::Linear, Graphics::MaterialDefaults::DefaultAo);

		if (!ao) return std::unexpected(std::move(ao).error());
		
		auto emission = material.emissionTexture
			? ImportTexture(material.emissionTexture.value(), Graphics::ColorSpace::Linear, root)
			: makePixel(Graphics::ChannelLayout::RGB, Graphics::ComponentType::Float32, Graphics::ColorSpace::Linear, emissionPixel);
		
		if (!emission) return std::unexpected(std::move(emission).error());

		auto normal = material.normalTexture
			? ImportTexture(material.normalTexture.value(), Graphics::ColorSpace::Linear, root)
			: makePixel(Graphics::ChannelLayout::RGB, Graphics::ComponentType::UInt8, Graphics::ColorSpace::Linear, Graphics::MaterialDefaults::DefaultNormal);

		if (!normal) return std::unexpected(std::move(normal).error());

		auto localShading = Graphics::Gl::ToLocalShadingChecked(material.surface);
		if (!localShading.second)
			spdlog::warn(
				"Material '{}' uses surface model, which is not supported with local illumination."
				"It will be treated as Unlit when shading via OpenGL. File path: {}",
				material.name,
				objPath.absolute.generic_string());
		
		auto globalShading = Graphics::Cuda::ToGlobalShadingChecked(material.surface);
		if (!globalShading.second)
			spdlog::warn(
				"Material '{}' uses surface model, which is not supported with global illumination."
				"It will be treated as Diffuse when shading via CUDA. File path: {}",
				material.name,
				objPath.absolute.generic_string());

		Material asset(
			material.surface,
			albedo.value(),
			specular.value(),
			shininess.value(),
			roughness.value(),
			metallic.value(),
			ao.value(),
			emission.value(),
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

		CORE_TRY_CONTEXT(parsed, Import::LoadObj(resolved.absolute), "Failed to parse OBJ file, file path: {}", absoluteStr);
		CORE_TRY(defaultMaterial, ImportDefaultMaterial());

		Model model{};
		model.parts.reserve(parsed.meshes.size());

		for (auto& mesh : parsed.meshes)
		{
			Handle<Material> materialHandle = defaultMaterial;

			if (mesh.materialIndex)
			{
				if (*mesh.materialIndex >= parsed.materials.size())
					return std::unexpected(Utils::Error("Invalid material index {}, file path: {}", *mesh.materialIndex, absoluteStr));

				CORE_TRY_CONTEXT(
					importedMaterial, 
					ImportMaterial(resolved, parsed.materials[*mesh.materialIndex]), 
					"Failed to import material for mesh {}, file path: {}", mesh.index, absoluteStr);

				materialHandle = importedMaterial;
			}

			CORE_TRY_CONTEXT(meshHandle, ImportMesh(resolved, std::move(mesh), mesh.index), "Failed to import mesh {}, file path: {}", mesh.index, absoluteStr);

			model.parts.push_back(ModelPart{
				.mesh = meshHandle,
				.material = materialHandle
			});
		}

		auto handle = m_Storage.Emplace(source, subkey, std::move(model));
		return handle.value();
	}

	std::expected<Handle<EnvironmentMap>, Utils::Error> Manager::ImportEnvironmentMap(const std::filesystem::path& path, Graphics::ColorSpace colorSpace)
	{
		CORE_TRY(resolvedPath, Utils::Path::ResolvePath(path, m_Root));

		std::string absoluteStr = resolvedPath.absolute.generic_string();
		std::string relativeStr = resolvedPath.relative.generic_string();

		Source source = SourcePath{ relativeStr };
		Subkey subkey = SubkeyNone{};

		auto cached = m_Storage.GetHandleByKey<EnvironmentMap>(source, subkey);
		if (cached)
			return cached.value();

		std::filesystem::path backgroundPath = resolvedPath.absolute / std::string(BackgroundFile);
		std::filesystem::path irradiancePath = resolvedPath.absolute / std::string(IrradianceDir);
		std::filesystem::path prefilteredPath = resolvedPath.absolute / std::string(PrefilteredDir);
		std::filesystem::path skyboxPath = resolvedPath.absolute / std::string(SkyboxDir);

		CORE_TRY_CONTEXT(backgroundImage, Import::LoadImage(backgroundPath, colorSpace), "Failed to load environment background, file path: {}", backgroundPath.string());

		if (!IsHdrTexture(backgroundImage.format))
			return std::unexpected(Utils::Error(
				"Environment background must be HDR (Float16/Float32), file path: {}", 
				backgroundPath.string()));

		CORE_TRY_CONTEXT(skybox, LoadCubemapTexture(skyboxPath, colorSpace), "Failed to load skybox cubemap, dir: {}", skyboxPath.string());
		CORE_TRY_CONTEXT(irradiance, LoadCubemapTexture(irradiancePath, colorSpace), "Failed to load irradiance cubemap, dir: {}", irradiancePath.string());
		CORE_TRY_CONTEXT(prefiltered, LoadCubemapTexture(prefilteredPath, colorSpace), "Failed to load prefiltered cubemap, dir: {}", prefilteredPath.string());
		
		EnvironmentMap asset(
			Graphics::Cpu::EnvironmentMap::Create(Graphics::Cpu::Texture::Create(std::move(backgroundImage))),
			Graphics::Gl::EnvironmentMap(std::move(skybox), std::move(irradiance), std::move(prefiltered)),
			Graphics::Cuda::EnvironmentMap{}
		);

		auto handle = m_Storage.Emplace(source, subkey, std::move(asset));
		return handle.value();
	}

	std::expected<Handle<ShaderProgram>, Utils::Error> Manager::ImportShaderProgram(std::span<std::pair<std::filesystem::path, Graphics::Gl::ShaderType>> shaderPaths)
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
			CORE_TRY(resolvedPath, Utils::Path::ResolvePath(path, m_Root));
			combinedPaths += resolvedPath.relative.generic_string() + ";";
			resolvedShaderPaths.emplace_back(std::move(resolvedPath), shaderType);
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
			CORE_TRY_CONTEXT(shaderSource, Import::LoadShader(path.absolute, shaderType), "Failed to load shader, file path: {}", path.absolute.string());
			CORE_TRY_CONTEXT(shader, Graphics::Gl::Shader::Create(std::move(shaderSource)), "Failed to create shader, file path: {}", path.absolute.string());
			shaders.emplace_back(std::move(shader));
		}

		CORE_TRY_CONTEXT(program, Graphics::Gl::ShaderProgram::Create(shaders), "Failed to create shader program");

		ShaderProgram programAsset(std::move(program));
		return m_Storage.Emplace(source, subkey, std::move(programAsset));
	}
}
