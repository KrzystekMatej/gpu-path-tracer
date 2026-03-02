#include "Assets/AssetManager.hpp"

namespace Core::Assets
{
	std::expected<AssetHandle<Model>, Error> AssetManager::ImportObj(const std::filesystem::path& path)
	{
		return std::unexpected(Error("OBJ import not implemented yet, file path: {}", path.string()));
	}

	std::expected<AssetHandle<Texture>, Error> AssetManager::ImportTexture(const std::filesystem::path& path, Graphics::ColorSpace colorSpace)
	{
		return std::unexpected(Error("Texture import not implemented yet, file path: {}", path.string()));
	}

	std::expected<AssetHandle<EnvironmentMap>, Error> AssetManager::ImportEnvironmentMap(const std::filesystem::path& path, Graphics::ColorSpace colorSpace)
	{
		return std::unexpected(Error("Environment map import not implemented yet, file path: {}", path.string()));
	}

	std::expected<AssetHandle<Mesh>, Error> AssetManager::ImportMesh(const std::filesystem::path& path, IO::ParsedMesh mesh, uint32_t index)
	{
		return std::unexpected(Error("Mesh import not implemented yet, file path: {}, mesh index: {}", path.string(), index));
	}

	std::expected<AssetHandle<Material>, Error> AssetManager::ImportMaterial(
		const std::filesystem::path& path,
		IO::ParsedMaterial material,
		const std::string name)
	{
		return std::unexpected(Error("Material import not implemented yet, file path: {}, material name: {}", path.string(), name));
	}

}