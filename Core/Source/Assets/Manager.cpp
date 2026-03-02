#include "Assets/Manager.hpp"

namespace Core::Assets
{
	std::expected<AssetHandle<Model>, Utils::Error> Manager::ImportObj(const std::filesystem::path& path)
	{
		return std::unexpected(Utils::Error("OBJ import not implemented yet, file path: {}", path.string()));
	}

	std::expected<AssetHandle<Texture>, Utils::Error> Manager::ImportTexture(const std::filesystem::path& path, Graphics::ColorSpace colorSpace)
	{
		return std::unexpected(Utils::Error("Texture import not implemented yet, file path: {}", path.string()));
	}

	std::expected<AssetHandle<EnvironmentMap>, Utils::Error> Manager::ImportEnvironmentMap(const std::filesystem::path& path, Graphics::ColorSpace colorSpace)
	{
		return std::unexpected(Utils::Error("Environment map import not implemented yet, file path: {}", path.string()));
	}

	std::expected<AssetHandle<Mesh>, Utils::Error> Manager::ImportMesh(const std::filesystem::path& path, IO::ParsedMesh mesh, uint32_t index)
	{
		return std::unexpected(Utils::Error("Mesh import not implemented yet, file path: {}, mesh index: {}", path.string(), index));
	}

	std::expected<AssetHandle<Material>, Utils::Error> Manager::ImportMaterial(
		const std::filesystem::path& path,
		IO::ParsedMaterial material,
		const std::string name)
	{
		return std::unexpected(Utils::Error("Material import not implemented yet, file path: {}, material name: {}", path.string(), name));
	}

}