#include "Assets/Manager.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "IO/ObjLoader.hpp"
#include "IO/ImageLoader.hpp"

namespace Core::Assets
{
	namespace
	{
		constexpr std::string_view DefaultMaterialSource = "default_material";
		constexpr std::string_view BackgroundFile = "background.exr";
		constexpr std::string_view IrradianceDir = "irradiance_map";
		constexpr std::string_view PrefilteredDir = "prefiltered_map";
		constexpr std::string_view SkyboxDir = "skybox";

		std::string NormalizePathString(const std::filesystem::path& p)
		{
			return p.lexically_normal().generic_string();
		}

		std::filesystem::path ResolveRelativeToFile(const std::filesystem::path& filePath, const std::string& maybeRelative)
		{
			std::filesystem::path p(maybeRelative);
			if (p.is_absolute())
				return p.lexically_normal();
			return (filePath.parent_path() / p).lexically_normal();
		}

		uint8_t NormalizedFloatToU8(float x)
		{
			if (!std::isfinite(x))
				x = 0.0f;
			x = std::clamp(x, 0.0f, 1.0f);
			return static_cast<uint8_t>(std::lround(x * 255.0f));
		}

		Key MakePathKey(AssetType type, std::string_view path, Subkey subkey)
		{
			return Key{ SourcePath{ path }, std::move(subkey), type };
		}

		Key MakePixelKey(const Graphics::PixelFormat& format, std::span<const uint8_t> data)
		{
			SourcePixel src{
				.externalFormat = static_cast<uint32_t>(format.layout),
				.pixelType = static_cast<uint32_t>(format.componentType),
				.internalFormat = static_cast<uint32_t>(format.colorSpace),
				.data = data
			};
			return Key{ src, SubkeyNone{}, AssetType::Texture };
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
			auto mipChain = IO::LoadCubemapMipChainFromFolder(folder, colorSpace);
			if (mipChain)
				return Core::Graphics::Gl::Texture::CreateCubemapFromMipmaps(mipChain.value());

			auto cubemap = IO::LoadCubemapFromFolder(folder, colorSpace, "");
			if (!cubemap)
				return std::unexpected(cubemap.error());

			return Core::Graphics::Gl::Texture::CreateCubemap(cubemap.value());
		}
	}

	std::expected<Manager, Utils::Error> Manager::Create()
	{
		Manager manager;
		auto defaultMat = manager.ImportDefaultMaterial();
		if (!defaultMat)
			return std::unexpected(Utils::Error(std::make_shared<Utils::Error>(defaultMat.error()), "Failed to import default material"));
		return manager;
	}

	std::expected<Handle<Texture>, Utils::Error> Manager::ImportPixelTexture(const Graphics::PixelFormat& format, std::span<const uint8_t> data)
	{
		Key key = MakePixelKey(format, data);

		auto cached = m_Storage.GetHandleByKey<Texture>(key);
		if (cached)
			return cached.value();

		IO::Image image{
			.width = 1,
			.height = 1,
			.format = format,
			.data = std::vector<uint8_t>(data.begin(), data.end())
		};

		auto glResult = Graphics::Gl::Texture::Create2D(image);
		if (!glResult)
			return std::unexpected(Utils::Error(std::make_shared<Utils::Error>(glResult.error()), "Failed to create GL texture from pixel"));

		Texture asset(
			Graphics::Cpu::Texture::Create(std::move(image)), 
			std::move(glResult.value()),
			Graphics::Cuda::Texture{});

		auto handle = m_Storage.Add(key, std::move(asset));
		if (!handle)
			return std::unexpected(handle.error());

		return handle.value();
	}

	std::expected<Handle<Material>, Utils::Error> Manager::ImportDefaultMaterial()
	{
		std::string src = std::string(DefaultMaterialSource);
		Key key = MakePathKey(AssetType::Material, src, SubkeyNone{});

		auto cached = m_Storage.GetHandleByKey<Material>(key);
		if (cached)
			return cached.value();

		Graphics::PixelFormat rgbSrgb{
			.layout = Graphics::ChannelLayout::RGB,
			.componentType = Graphics::ComponentType::UInt8,
			.colorSpace = Graphics::ColorSpace::SRGB
		};

		Graphics::PixelFormat rLinear{
			.layout = Graphics::ChannelLayout::R,
			.componentType = Graphics::ComponentType::UInt8,
			.colorSpace = Graphics::ColorSpace::Linear
		};

		Graphics::PixelFormat rgbLinear{
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
			Graphics::MaterialDefaults::DefaultShader,
			albedo.value(),
			roughness.value(),
			metallic.value(),
			ao.value(),
			normal.value());

		auto handle = m_Storage.Add(key, std::move(asset));
		if (!handle)
			return std::unexpected(handle.error());

		return handle.value();
	}

	std::expected<Handle<Texture>, Utils::Error> Manager::ImportTexture(const std::filesystem::path& path, Graphics::ColorSpace colorSpace)
	{
		std::string pathStr = NormalizePathString(path);
		Key key = MakePathKey(
			AssetType::Texture,
			pathStr,
			SubkeyIndex{ static_cast<uint32_t>(colorSpace) }
		);

		auto cached = m_Storage.GetHandleByKey<Texture>(key);
		if (cached)
			return cached.value();

		auto imageResult = IO::LoadImage(path, colorSpace);
		if (!imageResult)
			return std::unexpected(Utils::Error(std::make_shared<Utils::Error>(imageResult.error()), "Failed to load image, file path: {}", pathStr));

		IO::Image image = std::move(imageResult.value());

		auto glResult = Graphics::Gl::Texture::Create2D(image);
		if (!glResult)
			return std::unexpected(Utils::Error(std::make_shared<Utils::Error>(glResult.error()), "Failed to create GL texture, file path: {}", pathStr));

		Texture asset(
			Graphics::Cpu::Texture::Create(std::move(image)), 
			std::move(glResult.value()), 
			Graphics::Cuda::Texture{});

		auto handle = m_Storage.Add(key, std::move(asset));
		if (!handle)
			return std::unexpected(handle.error());

		return handle.value();
	}

	std::expected<Handle<Mesh>, Utils::Error> Manager::ImportMesh(const std::filesystem::path& path, IO::ParsedMesh mesh, uint32_t index)
	{
		std::string objStr = NormalizePathString(path);
		Key key = MakePathKey(AssetType::Mesh, objStr, SubkeyIndex{ index });

		auto cached = m_Storage.GetHandleByKey<Mesh>(key);
		if (cached)
			return cached.value();

		auto glResult = Graphics::Gl::Mesh::Create(mesh);
		if (!glResult)
			return std::unexpected(Utils::Error(std::make_shared<Utils::Error>(glResult.error()), "Failed to create GL mesh, obj path: {}", objStr));

		Mesh asset(Graphics::Cpu::Mesh::Create(std::move(mesh)), std::move(glResult.value()));

		auto handle = m_Storage.Add(key, std::move(asset));
		if (!handle)
			return std::unexpected(handle.error());

		return handle.value();
	}

	std::expected<Handle<Material>, Utils::Error> Manager::ImportMaterial(const std::filesystem::path& objPath, const IO::ParsedMaterial& material)
	{
		std::string objStr = NormalizePathString(objPath);
		Key key = MakePathKey(AssetType::Material, objStr, SubkeyName{ material.name });

		auto cached = m_Storage.GetHandleByKey<Material>(key);
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

		auto albedo = material.albedoTexture
			? ImportTexture(ResolveRelativeToFile(objPath, *material.albedoTexture), Graphics::ColorSpace::SRGB)
			: makeRgb(Graphics::ColorSpace::SRGB, albedoPixel);

		if (!albedo) return std::unexpected(albedo.error());

		auto roughness = material.roughnessTexture
			? ImportTexture(ResolveRelativeToFile(objPath, *material.roughnessTexture), Graphics::ColorSpace::Linear)
			: makeR(roughnessPixel);

		if (!roughness) return std::unexpected(roughness.error());

		auto metallic = material.metallicTexture
			? ImportTexture(ResolveRelativeToFile(objPath, *material.metallicTexture), Graphics::ColorSpace::Linear)
			: makeR(metallicPixel);

		if (!metallic) return std::unexpected(metallic.error());

		auto ao = material.aoTexture
			? ImportTexture(ResolveRelativeToFile(objPath, *material.aoTexture), Graphics::ColorSpace::Linear)
			: makeR(Graphics::MaterialDefaults::DefaultAo);

		if (!ao) return std::unexpected(ao.error());

		auto normal = material.normalTexture
			? ImportTexture(ResolveRelativeToFile(objPath, *material.normalTexture), Graphics::ColorSpace::Linear)
			: makeRgb(Graphics::ColorSpace::Linear, Graphics::MaterialDefaults::DefaultNormal);

		if (!normal) return std::unexpected(normal.error());

		Material asset(
			material.shader,
			albedo.value(),
			roughness.value(),
			metallic.value(),
			ao.value(),
			normal.value());

		auto handle = m_Storage.Add(key, std::move(asset));
		if (!handle)
			return std::unexpected(handle.error());

		return handle.value();
	}

	std::expected<Handle<Model>, Utils::Error> Manager::ImportObj(const std::filesystem::path& path)
	{
		std::string objStr = NormalizePathString(path);
		Key key = MakePathKey(AssetType::Model, objStr, SubkeyNone{});

		auto cached = m_Storage.GetHandleByKey<Model>(key);
		if (cached)
			return cached.value();

		auto parsedResult = IO::LoadObj(path);
		if (!parsedResult)
			return std::unexpected(Utils::Error(
				std::make_shared<Utils::Error>(parsedResult.error()), 
				"Failed to load OBJ, file path: {}", 
				objStr));

		IO::ParsedModel parsed = std::move(parsedResult.value());

		auto defaultMat = ImportDefaultMaterial();
		if (!defaultMat)
			return std::unexpected(defaultMat.error());

		Model model{};
		model.parts.reserve(parsed.meshes.size());

		for (auto& m : parsed.meshes)
		{
			Handle<Material> matHandle = defaultMat.value();

			if (m.materialIndex)
			{
				if (*m.materialIndex >= parsed.materials.size())
					return std::unexpected(Utils::Error("Invalid material index {}, file path: {}", *m.materialIndex, objStr));

				auto materialHandle = ImportMaterial(path, parsed.materials[*m.materialIndex]);
				if (!materialHandle)
					return std::unexpected(materialHandle.error());

				matHandle = materialHandle.value();
			}

			auto meshHandle = ImportMesh(path, std::move(m), m.index);
			if (!meshHandle)
				return std::unexpected(meshHandle.error());

			model.parts.push_back(ModelPart{
				.mesh = meshHandle.value(),
				.material = matHandle
			});
		}

		auto handle = m_Storage.Add(key, std::move(model));
		if (!handle)
			return std::unexpected(handle.error());

		return handle.value();
	}

	std::expected<Handle<EnvironmentMap>, Utils::Error> Manager::ImportEnvironmentMap(const std::filesystem::path& path, Graphics::ColorSpace colorSpace)
	{
		std::string dirStr = NormalizePathString(path);
		Key key = MakePathKey(
			AssetType::EnvironmentMap,
			dirStr,
			SubkeyIndex{ static_cast<uint32_t>(colorSpace) }
		);

		auto cached = m_Storage.GetHandleByKey<EnvironmentMap>(key);
		if (cached)
			return cached.value();

		std::filesystem::path backgroundPath = path / std::string(BackgroundFile);
		std::filesystem::path irradiancePath = path / std::string(IrradianceDir);
		std::filesystem::path prefilteredPath = path / std::string(PrefilteredDir);
		std::filesystem::path skyboxPath = path / std::string(SkyboxDir);

		auto backgroundResult = IO::LoadImage(backgroundPath, colorSpace);
		if (!backgroundResult)
			return std::unexpected(Utils::Error(
				std::make_shared<Utils::Error>(backgroundResult.error()), 
				"Failed to load environment background, file path: {}", 
				backgroundPath.string()));

		IO::Image backgroundImage = std::move(backgroundResult.value());

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

		auto handle = m_Storage.Add(key, std::move(asset));
		if (!handle)
			return std::unexpected(handle.error());

		return handle.value();
	}
}
