#pragma once
#include <expected>
#include <filesystem>
#include "Assets/Types.hpp"
#include "Assets/Storage.hpp"
#include "IO/Model.hpp"
#include "Utils/Path.hpp"

namespace Core::Assets
{
	class Manager
	{
	public:
		Manager(const Manager&) = delete;
		Manager& operator=(const Manager&) = delete;
		Manager(Manager&&) noexcept = default;
		Manager& operator=(Manager&&) noexcept = default;

		static std::expected<Manager, Utils::Error> Create(std::filesystem::path assetsPath);

		const Storage& GetStorage() const { return m_Storage; }
		const std::filesystem::path& GetRootPath() const { return m_Root; }

		std::expected<Handle<Model>, Utils::Error> ImportObj(const std::filesystem::path& path);
		std::expected<Handle<Texture>, Utils::Error> ImportTexture(
			const std::filesystem::path& path, 
			Graphics::ColorSpace colorSpace, 
			std::optional<std::filesystem::path> root = std::nullopt);
		std::expected<Handle<EnvironmentMap>, Utils::Error> ImportEnvironmentMap(const std::filesystem::path& path, Graphics::ColorSpace colorSpace);
		std::expected<Handle<ShaderProgram>, Utils::Error> ImportShaderProgram(
			std::span<std::pair<std::filesystem::path, Graphics::Gl::ShaderType>> shaderPaths);
	private:
		Manager() = default;

		std::expected<Handle<Mesh>, Utils::Error> ImportMesh(const Utils::Path::ResolvedPath& objPath, IO::ParsedMesh mesh, uint32_t index);
		std::expected<Handle<Material>, Utils::Error> ImportMaterial(const Utils::Path::ResolvedPath& objPath, const IO::ParsedMaterial& material);

		std::expected<Handle<Material>, Utils::Error> ImportDefaultMaterial();
		std::expected<Handle<Texture>, Utils::Error> ImportPixelTexture(const Graphics::PixelFormat& format, std::span<const uint8_t> data);

		Storage m_Storage;
		std::filesystem::path m_Root;
	};
}
