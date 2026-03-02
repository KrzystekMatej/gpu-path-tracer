#pragma once
#include <expected>
#include <filesystem>
#include "Assets/Types.hpp"
#include "Assets/Storage.hpp"
#include "IO/Model.hpp"

namespace Core::Assets
{
	class Manager
	{
	public:
		Manager() = default;
		Manager(const Manager&) = delete;
		Manager& operator=(const Manager&) = delete;
		Manager(Manager&&) noexcept = default;
		Manager& operator=(Manager&&) noexcept = default;

		const Storage& GetStorage() const { return m_Storage; }
		std::expected<AssetHandle<Model>, Utils::Error> ImportObj(const std::filesystem::path& path);
		std::expected<AssetHandle<Texture>, Utils::Error> ImportTexture(const std::filesystem::path& path, Graphics::ColorSpace colorSpace);
		std::expected<AssetHandle<EnvironmentMap>, Utils::Error> ImportEnvironmentMap(const std::filesystem::path& path, Graphics::ColorSpace colorSpace);
	private:
		std::expected<AssetHandle<Mesh>, Utils::Error> ImportMesh(const std::filesystem::path& path, IO::ParsedMesh mesh, uint32_t index);
		std::expected<AssetHandle<Material>, Utils::Error> ImportMaterial(const std::filesystem::path& path, IO::ParsedMaterial material, const std::string name);


		Storage m_Storage;
	};


}