#pragma once
#include <expected>
#include <filesystem>
#include <Core/Assets/Types.hpp>
#include <Core/Assets/Storage.hpp>
#include <Core/Import/Prefabs/ScenePrefab.hpp>
#include <Core/Utils/Path.hpp>
#include <Core/Graphics/Common/Formats.hpp>

namespace Core::Assets
{
	struct MeshCacheAccessors
	{
		Key<Mesh> mesh;
		std::optional<Utils::Guid> indicesId;
		Utils::Guid positionsId;
		std::optional<Utils::Guid> normalsId;
		std::optional<Utils::Guid> tangentsId;
		std::optional<Utils::Guid> uvsId;
	};
	
	struct ResolvedMaterial
	{
		Graphics::SurfaceModel surface;
		bool flipTextures;

		std::array<float, 3> color;
		std::array<float, 3> specular;
		float shininess;
		float roughness;
		float metallic;
		std::array<float, 3> emission;
		float ior;
		float transmission;
		float opacity;

		std::optional<Utils::Path::ResolvedPath> colorTexture;
		std::optional<Utils::Path::ResolvedPath> specularTexture;
		std::optional<Utils::Path::ResolvedPath> shininessTexture;
		std::optional<Utils::Path::ResolvedPath> occlusionTexture;
		std::optional<Utils::Path::ResolvedPath> roughnessTexture;
		std::optional<Utils::Path::ResolvedPath> metallicTexture;
		std::optional<Utils::Path::ResolvedPath> rmTexture;
		std::optional<Utils::Path::ResolvedPath> ormTexture;
		std::optional<Utils::Path::ResolvedPath> rmoTexture;
		std::optional<Utils::Path::ResolvedPath> emissionTexture;
		std::optional<Utils::Path::ResolvedPath> normalTexture;
	};

	class Manager
	{
	public:
		static constexpr std::string_view DefaultMaterialSource = "default_material";
		static constexpr std::string_view BackgroundFile = "background.exr";
		static constexpr std::string_view IrradianceDir = "irradiance_map";
		static constexpr std::string_view PrefilteredDir = "prefiltered_map";
		static constexpr std::string_view SkyboxDir = "skybox";
		
		Manager(const Manager&) = delete;
		Manager& operator=(const Manager&) = delete;
		Manager(Manager&&) noexcept = default;
		Manager& operator=(Manager&&) noexcept = default;

		static std::expected<Manager, Utils::Error> Create(std::filesystem::path assetsPath);

		const Storage& GetStorage() const { return m_Storage; }
		const std::filesystem::path& GetRootPath() const { return m_Root; }
		Handle<Material> GetDefaultMaterial() const;

		std::expected<Handle<ScenePrefab>, Utils::Error> ImportScenePrefabFromFile(const std::filesystem::path& path);
		std::expected<Handle<EnvironmentMap>, Utils::Error> ImportEnvironmentMap(const std::filesystem::path& path, Graphics::ColorSpace colorSpace);
		std::expected<Handle<ShaderProgram>, Utils::Error> ImportShaderProgram(std::span<std::pair<std::filesystem::path, Graphics::Gl::ShaderType>> shaderPaths);
		void ClearBufferCache() { m_BufferCache.clear(); }
	private:
		Manager() = default;

		std::expected<Handle<Texture>, Utils::Error> ImportTexture(
			const Utils::Path::ResolvedPath& resolvedPath,
			Graphics::ColorSpace colorSpace, 
			bool flipVertically = false);
		std::expected<Handle<Texture>, Utils::Error> ImportFileOrPixelTexture(
			const std::optional<Utils::Path::ResolvedPath>& path,	
			const Graphics::PixelFormat& defaultFormat, 
			std::span<const uint8_t> defaultValue,
			bool flipVertically);
		std::expected<void, Utils::Error> ImportMeshBuffer(
			std::span<uint8_t> destinationBufferBytes,
			const Import::BufferView& destinationView, 
			const Import::BufferAccessor& accessor, 
			const Import::ScenePrefab& source, 
			Utils::Guid bufferId);
		std::expected<Utils::Guid, Utils::Error> MakeMeshBufferId(std::string_view path, const std::filesystem::path& parentPath, const Import::BufferAccessor& accessor, const Import::ScenePrefab& source);
		void AddBufferViewToMeshKey(KeyBuilder<Mesh>& builder, const Import::BufferAccessor& accessor);
		std::expected<MeshCacheAccessors, Utils::Error> MakeMeshCacheAccessors(std::string_view path, const std::filesystem::path& parentPath, const Import::MeshAccessor& accessor, const Import::ScenePrefab& source);
		std::expected<Import::MeshPrimitive, Utils::Error> ComposeMesh(const Import::MeshAccessor& accessor, const Import::ScenePrefab& source, const MeshCacheAccessors& cacheAccessors);
		std::expected<Handle<Mesh>, Utils::Error> ImportMesh(const Utils::Path::ResolvedPath& path, const std::filesystem::path& parentPath, const Import::ScenePrefab& prefab, Import::MeshAccessor mesh, uint32_t index);
		std::expected<Handle<Material>, Utils::Error> ImportMaterial(const Utils::Path::ResolvedPath& path, const std::filesystem::path& parentPath, const Import::Material& material, uint32_t index);
		std::expected<Handle<Material>, Utils::Error> ImportDefaultMaterial();
		std::expected<Handle<Texture>, Utils::Error> ImportOrmTexture(const ResolvedMaterial& resolvedMaterial);
		std::expected<Import::Media::Image, Utils::Error> LoadTextureImage(const std::filesystem::path& path, Graphics::ColorSpace colorSpace, std::optional<std::filesystem::path> root = std::nullopt);
		std::expected<Handle<Texture>, Utils::Error> ImportPixelTexture(const Graphics::PixelFormat& format, std::span<const uint8_t> data);
		std::expected<std::optional<Utils::Path::ResolvedPath>, Utils::Error> ResolveTexturePath(const std::optional<std::filesystem::path>& texturePath, const std::filesystem::path& root);
		std::expected<ResolvedMaterial, Utils::Error> ResolveMaterial(const Utils::Path::ResolvedPath& path, const std::filesystem::path& parentPath, const Import::Material& materialSource);
		std::expected<Utils::Path::ResolvedPath, Utils::Error> ResolveFilePath(const std::filesystem::path& path, const std::optional<std::filesystem::path>& root = std::nullopt);

		Storage m_Storage;
		std::filesystem::path m_Root;
		std::unordered_map<Core::Utils::Guid, std::vector<uint8_t>> m_BufferCache;
	};
}
