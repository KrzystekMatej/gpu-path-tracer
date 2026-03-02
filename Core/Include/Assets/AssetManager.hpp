#pragma once
#include <expected>
#include <filesystem>
#include "Assets/Types.hpp"
#include "Assets/AssetStorage.hpp"
#include "IO/Model.hpp"

namespace Core::Assets
{
	class AssetManager
	{
	public:
		AssetManager() = default;
		AssetManager(const AssetManager&) = delete;
		AssetManager& operator=(const AssetManager&) = delete;
		AssetManager(AssetManager&&) noexcept = default;
		AssetManager& operator=(AssetManager&&) noexcept = default;

		const AssetStorage& GetStorage() const { return m_Storage; }
		std::expected<AssetHandle<Model>, Error> ImportObj(const std::filesystem::path& path);
		std::expected<AssetHandle<Texture>, Error> ImportTexture(const std::filesystem::path& path, Graphics::ColorSpace colorSpace);
		std::expected<AssetHandle<EnvironmentMap>, Error> ImportEnvironmentMap(const std::filesystem::path& path, Graphics::ColorSpace colorSpace);
	private:
		std::expected<AssetHandle<Mesh>, Error> ImportMesh(const std::filesystem::path& path, IO::ParsedMesh mesh, uint32_t index);
		std::expected<AssetHandle<Material>, Error> ImportMaterial(const std::filesystem::path& path, IO::ParsedMaterial material, const std::string name);


		AssetStorage m_Storage;
	};


}