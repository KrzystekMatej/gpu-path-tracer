#include <Core/Assets/Manager.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <functional>
#include <ranges>

#include <Core/Utils/Hash.hpp>
#include <Core/Import/Prefabs/GlTFLoader.hpp>
#include <Core/Utils/Types.hpp>
#include <Core/Import/Prefabs/ObjLoader.hpp>
#include <Core/Import/Media/ImageIO.hpp>
#include <Core/Import/ShaderLoader.hpp>
#include <Core/Graphics/Gl/Material.hpp>
#include <Core/Graphics/Cuda/PathTracing/Material.hpp>
#include <Core/Import/Media/ImageUtils.hpp>
#include <Core/Import/Media/OrmHelpers.hpp>
#include <Core/Utils/NumberTypes.hpp>
#include <Core/Import/Buffers/BufferUtils.hpp>
#include <Core/Utils/Math/TangentSpace.hpp>

namespace Core::Assets
{
	namespace
	{
		
		std::array<uint8_t, 3> RgbToU8(const float color[3])
		{
			return {
				Utils::NumberTypes::NormalizedFloatToU8(color[0]),
				Utils::NumberTypes::NormalizedFloatToU8(color[1]),
				Utils::NumberTypes::NormalizedFloatToU8(color[2])
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
		
		std::array<uint8_t, 3> LinearRgbToSrgbU8(std::array<float, 3> color)
		{
			return {
				LinearNormalizedFloatToSrgbU8(color[0]),
				LinearNormalizedFloatToSrgbU8(color[1]),
				LinearNormalizedFloatToSrgbU8(color[2])
			};
		}

		bool IsHdrTexture(const Graphics::PixelFormat& format)
		{
			return format.componentType == Graphics::ComponentType::Float16 || format.componentType == Graphics::ComponentType::Float32;
		}

		Key<Texture> MakePixelTextureKey(const Graphics::PixelFormat& format, std::span<const uint8_t> value)
		{
			KeyBuilder<Texture> builder = KeyBuilder<Texture>();
			builder.Add(format.layout);
			builder.Add(format.componentType);
			builder.Add(format.colorSpace);
			uint64_t length = value.size();
			builder.Add(length);
			builder.Add(value);
			return builder.Finish("pixel_texture");
		}

		std::expected<Import::Media::Image, Utils::Error> CreateOrmTextureImage(const ResolvedMaterial& material)
		{
			Import::Media::Image ormImage;
			constexpr Graphics::ColorSpace colorSpace = Graphics::ColorSpace::Linear;
			
			if (material.ormTexture)
			{
				std::filesystem::path resolvedPath = material.ormTexture.value().absolute;
				CORE_TRY_ASSIGN_CONTEXT(ormImage, Import::Media::LoadImage(resolvedPath, colorSpace), "Failed to load ORM texture image, file path: {}", resolvedPath.string());
			}
			else if (material.rmoTexture)
			{
				std::filesystem::path resolvedPath = material.rmoTexture.value().absolute;
				CORE_TRY_ASSIGN_CONTEXT(ormImage, Import::Media::LoadImage(resolvedPath, colorSpace), "Failed to load RMO texture image, file path: {}", resolvedPath.string());
				CORE_TRY_DISCARD_CONTEXT(Import::Media::ConvertRmoToOrm(ormImage), "Failed to convert RMO texture to ORM texture, file path: {}", resolvedPath.string());
			}
			else if (material.rmTexture)
			{
				std::filesystem::path rmPath = material.rmTexture.value().absolute;
				CORE_TRY_CONTEXT(rmImage, Import::Media::LoadImage(rmPath, colorSpace), "Failed to load RM texture image, file path: {}", rmPath.string());
				std::optional<Import::Media::Image> occlusionImage;
				if (material.occlusionTexture)
				{
					std::filesystem::path occlusionPath = material.occlusionTexture.value().absolute;
					CORE_TRY_ASSIGN_CONTEXT(occlusionImage, Import::Media::LoadImage(occlusionPath, colorSpace), "Failed to load occlusion texture image, file path: {}", occlusionPath.string());
				}

				CORE_TRY_ASSIGN_CONTEXT(ormImage, Import::Media::PackOrmTexture(occlusionImage, Graphics::MaterialDefaults::Occlusion, rmImage), "Failed to pack ORM texture, file path: {}", rmPath.string());
			}
			else if (material.occlusionTexture || material.roughnessTexture || material.metallicTexture)
			{
				std::optional<Import::Media::Image> occlusionImage;
				if (material.occlusionTexture)
				{
					std::filesystem::path occlusionPath = material.occlusionTexture.value().absolute;
					CORE_TRY_ASSIGN_CONTEXT(occlusionImage, Import::Media::LoadImage(occlusionPath, colorSpace), "Failed to load occlusion texture image, file path: {}", occlusionPath.string());
				}
				
				std::optional<Import::Media::Image> roughnessImage;
				if (material.roughnessTexture)
				{
					std::filesystem::path roughnessPath = material.roughnessTexture.value().absolute;
					CORE_TRY_ASSIGN_CONTEXT(roughnessImage, Import::Media::LoadImage(roughnessPath, colorSpace), "Failed to load roughness texture image, file path: {}", roughnessPath.string());
				}

				std::optional<Import::Media::Image> metallicImage;
				if (material.metallicTexture)
				{
					std::filesystem::path metallicPath = material.metallicTexture.value().absolute;
					CORE_TRY_ASSIGN_CONTEXT(metallicImage, Import::Media::LoadImage(metallicPath, colorSpace), "Failed to load metallic texture image, file path: {}", metallicPath.string());
				}

				CORE_TRY_ASSIGN_CONTEXT(
					ormImage, 
					Import::Media::PackOrmTexture(
						occlusionImage,
						Graphics::MaterialDefaults::Occlusion,
						roughnessImage, 
						material.roughness,
						metallicImage, 
						material.metallic),
					"Failed to pack ORM texture");
			}
			else
			{
				std::array<uint8_t, 3> ormPixel = {
					Utils::NumberTypes::NormalizedFloatToU8(Graphics::MaterialDefaults::Occlusion),
					Utils::NumberTypes::NormalizedFloatToU8(material.roughness),
					Utils::NumberTypes::NormalizedFloatToU8(material.metallic),
				};

				ormImage = std::move(Import::Media::Image {
					.width = 1,
					.height = 1,
					.format = {
						.layout = Graphics::ChannelLayout::RGB,
						.componentType = Graphics::ComponentType::UInt8,
						.colorSpace = colorSpace
					},
					.data = { ormPixel.begin(), ormPixel.end() }
				});
			}

			if (material.flipTextures)
				ormImage = std::move(Import::Media::FlipVertically(ormImage));

			return ormImage;
		}

		Key<Texture> MakeOrmTextureKey(const ResolvedMaterial& material)
		{
			KeyBuilder<Texture> builder;
			
			if (material.ormTexture)
			{
				builder.Add(material.ormTexture.value().relative.generic_string());
			}
			else if (material.rmoTexture)
			{
				builder.Add(material.rmoTexture.value().relative.generic_string());
			}
			else if (material.rmTexture)
			{
				builder.Add(material.rmTexture.value().relative.generic_string());
				if (material.occlusionTexture)
					builder.Add(material.occlusionTexture.value().relative.generic_string());
				else
					builder.Add(Utils::NumberTypes::NormalizedFloatToU8(Graphics::MaterialDefaults::Occlusion));
			}
			else if (material.occlusionTexture || material.roughnessTexture || material.metallicTexture)
			{
				if (material.occlusionTexture)
					builder.Add(material.occlusionTexture.value().relative.generic_string());
				else
					builder.Add(Utils::NumberTypes::NormalizedFloatToU8(Graphics::MaterialDefaults::Occlusion));

				if (material.roughnessTexture)
					builder.Add(material.roughnessTexture.value().relative.generic_string());
				else
					builder.Add(Utils::NumberTypes::NormalizedFloatToU8(material.roughness));

				if (material.metallicTexture)
					builder.Add(material.metallicTexture.value().relative.generic_string());
				else
					builder.Add(Utils::NumberTypes::NormalizedFloatToU8(material.metallic));
			}
			else
			{
				std::array<uint8_t, 3> ormPixel = {
					Utils::NumberTypes::NormalizedFloatToU8(Graphics::MaterialDefaults::Occlusion),
					Utils::NumberTypes::NormalizedFloatToU8(material.roughness),
					Utils::NumberTypes::NormalizedFloatToU8(material.metallic),
				};

				Graphics::PixelFormat pixelFormat = {
					.layout = Graphics::ChannelLayout::RGB,
					.componentType = Graphics::ComponentType::UInt8,
					.colorSpace = Graphics::ColorSpace::Linear
				};
				
				return MakePixelTextureKey(pixelFormat, ormPixel);
			}

			return builder.Finish("orm_texture");
		}

		Graphics::SurfaceModel ResolveSurfaceModel(const Import::Material& material)
		{
			if (material.surface.has_value())
				return material.surface.value();

			if (material.roughness > 0.0f || material.metallic > 0.0f || material.ormTexture.has_value() || material.rmoTexture.has_value() 
				|| material.rmTexture.has_value() || material.roughnessTexture.has_value() || material.metallicTexture.has_value())
				return Graphics::SurfaceModel::Microfacet;
			
			if ((material.specular[0] > 0.0f || material.specular[1] > 0.0f || material.specular[2] > 0.0f || material.specularTexture.has_value()) &&
				(material.shininess > 0.0f || material.shininessTexture.has_value()))
				return Graphics::SurfaceModel::Phong;

			if ((material.specular[0] > 0.0f || material.specular[1] > 0.0f || material.specular[2] > 0.0f || material.specularTexture.has_value()) &&
				!(material.shininess > 0.0f || material.shininessTexture.has_value()))
				return Graphics::SurfaceModel::Mirror;
			
			if (material.emission[0] > 0.0f || material.emission[1] > 0.0f || material.emission[2] > 0.0f || material.emissionTexture.has_value())
				return Graphics::SurfaceModel::Emissive;

			return Graphics::SurfaceModel::Diffuse;
		}
		
		template<Asset T>
		Key<T> MakePathSourcedKey(std::string_view path, Utils::Text::StringLiteral debugName)
		{
			KeyBuilder<T> builder = KeyBuilder<T>();
			builder.Add(path);
			return builder.Finish(debugName);
		}
	


		std::expected<Core::Graphics::Gl::Texture, Utils::Error> LoadCubemapTexture(const std::filesystem::path& folder, Graphics::ColorSpace colorSpace)
		{
			auto mipChain = Import::Media::LoadCubemapMipChainFromFolder(folder, colorSpace);
			if (mipChain)
				return Core::Graphics::Gl::Texture::CreateCubemapFromMipmaps(mipChain.value());
			auto cubemap = Import::Media::LoadCubemapFromFolder(folder, colorSpace, "");
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
	
	Handle<Material> Manager::GetDefaultMaterial() const
	{
		Key<Material> key = MakePathSourcedKey<Material>("default_material", DefaultMaterialSource);
		auto cached = m_Storage.GetHandleByKey<Material>(key);
		assert(cached.has_value() && "Default material not present in storage");
		return cached.value();
	}

	std::expected<Handle<ScenePrefab>, Utils::Error> Manager::ImportScenePrefabFromFile(const std::filesystem::path& path)
	{
		CORE_TRY_CONTEXT(resolvedPath, ResolveFilePath(path, std::nullopt), "Failed to resolve scene prefab file path: {}", path.string());
		std::string absoluteStr = resolvedPath.absolute.generic_string();
		std::string relativeStr = resolvedPath.relative.generic_string();

		Key<ScenePrefab> key = MakePathSourcedKey<ScenePrefab>(relativeStr, "scene_prefab_from_file");
		if (auto cached = m_Storage.GetHandleByKey<ScenePrefab>(key); cached)
			return cached.value();
		
		std::string extension = resolvedPath.absolute.extension().string();
		Import::ScenePrefab parsed;

		if (extension == ".obj")
		{
			CORE_TRY_ASSIGN_CONTEXT(parsed, Import::LoadObj(resolvedPath.absolute), "Failed to parse OBJ file, file path: {}", absoluteStr);
		}
		else if (extension == ".gltf" || extension == ".glb")
		{
			CORE_TRY_ASSIGN_CONTEXT(parsed, Import::LoadGlTF(resolvedPath.absolute), "Failed to parse GLTF file, file path: {}", absoluteStr);
		}
		else
		{
			return std::unexpected(Utils::Error("Unsupported scene prefab file extension: {}", extension));
		}

		Handle<Material> defaultMaterial = GetDefaultMaterial();

		ScenePrefab prefab;
		prefab.name = resolvedPath.absolute.stem().string();
		prefab.nodes.reserve(parsed.nodes.size());
		prefab.models.reserve(parsed.meshes.size());
		prefab.roots = std::move(parsed.roots);
		
		for (auto& node : parsed.nodes)
		{
			prefab.nodes.push_back(SceneNode{
				.name = node.name,
				.translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]),
				.rotation = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]),
				.scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]),
				.model = node.mesh,
				.children = std::move(node.children)
			});
		}
		
		std::filesystem::path parentPath = resolvedPath.absolute.parent_path();

		for (auto& mesh : parsed.meshes)
		{
			Model model;
			model.primitives.reserve(mesh.primitives.size());

			for (uint32_t primitiveIndex : mesh.primitives)
			{
				assert(primitiveIndex < parsed.meshPrimitivesAccessors.size());
				auto& primitive = parsed.meshPrimitivesAccessors[primitiveIndex];
				
				Handle<Material> materialHandle = defaultMaterial;

				if (primitive.material)
				{
					assert(*primitive.material < parsed.materials.size());// importer thing?
					uint32_t materialIndex = primitive.material.value();

					CORE_TRY_ASSIGN_CONTEXT(
						materialHandle,
						ImportMaterial(resolvedPath, parentPath, parsed.materials[materialIndex], materialIndex),  
						"Failed to import material for mesh {}, file path: {}", 
						primitiveIndex, 
						absoluteStr);
				}

				CORE_TRY_CONTEXT(meshHandle, ImportMesh(resolvedPath, parentPath, parsed, primitive, primitiveIndex), "Failed to import mesh {}, file path: {}", primitiveIndex, absoluteStr);
				model.primitives.push_back(ModelPrimitive{ .mesh = meshHandle, .material = materialHandle });
			}

			prefab.models.push_back(std::move(model));
		}

		return m_Storage.Emplace(key, std::move(prefab)).value();
	}
	
	std::expected<Handle<Texture>, Utils::Error> Manager::ImportTexture(
		const Utils::Path::ResolvedPath& resolvedPath,
		Graphics::ColorSpace colorSpace, 
		bool flipVertically)
	{
		std::string absoluteStr = resolvedPath.absolute.generic_string();
		std::string relativeStr = resolvedPath.relative.generic_string();

		Key<Texture> key = MakePathSourcedKey<Texture>(relativeStr, "2d_image_texture");
		if (auto cached = m_Storage.GetHandleByKey<Texture>(key); cached)
			return cached.value();

		CORE_TRY_CONTEXT(image, Import::Media::LoadImage(resolvedPath.absolute, colorSpace), "Failed to load texture image, file path: {}", absoluteStr);
		if (flipVertically)
			image = Import::Media::FlipVertically(std::move(image));

		CORE_TRY_CONTEXT(glTexture, Graphics::Gl::Texture::Create2D(image), "Failed to create GL texture, file path: {}", absoluteStr);
		CORE_TRY_CONTEXT(cudaTexture, Graphics::Cuda::Texture::Create2D(image), "Failed to create CUDA texture, file path: {}", absoluteStr);
		Graphics::Cpu::Texture cpuTexture = Graphics::Cpu::Texture::Create(std::move(image));

		Texture asset(std::move(cpuTexture), std::move(glTexture), std::move(cudaTexture));
		return m_Storage.Emplace(key, std::move(asset));
	}

	std::expected<Handle<EnvironmentMap>, Utils::Error> Manager::ImportEnvironmentMap(const std::filesystem::path& path, Graphics::ColorSpace colorSpace)
	{
		CORE_TRY_CONTEXT(resolvedPath, ResolveFilePath(path, std::nullopt), "Failed to resolve environment map path: {}", path.string());
		std::string absoluteStr = resolvedPath.absolute.generic_string();
		std::string relativeStr = resolvedPath.relative.generic_string();

		Key<EnvironmentMap> key = MakePathSourcedKey<EnvironmentMap>(relativeStr, "environment_map");

		if (auto cached = m_Storage.GetHandleByKey<EnvironmentMap>(key); cached)
			return cached.value();

		std::filesystem::path backgroundPath = resolvedPath.absolute / std::string(BackgroundFile);
		std::filesystem::path irradiancePath = resolvedPath.absolute / std::string(IrradianceDir);
		std::filesystem::path prefilteredPath = resolvedPath.absolute / std::string(PrefilteredDir);
		std::filesystem::path skyboxPath = resolvedPath.absolute / std::string(SkyboxDir);

		CORE_TRY_CONTEXT(backgroundImage, Import::Media::LoadImage(backgroundPath, colorSpace), "Failed to load environment background, file path: {}", backgroundPath.string());

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

		return m_Storage.Emplace(key, std::move(asset));
	}

	std::expected<Handle<ShaderProgram>, Utils::Error> Manager::ImportShaderProgram(std::span<std::pair<std::filesystem::path, Graphics::Gl::ShaderType>> shaderPaths)
	{
		
		std::ranges::sort(shaderPaths, [](const auto& a, const auto& b)
		{
			return a.second < b.second;
		});
		
		std::vector<std::pair<Utils::Path::ResolvedPath, Graphics::Gl::ShaderType>> resolvedShaderPaths;
		resolvedShaderPaths.reserve(shaderPaths.size());
		KeyBuilder<ShaderProgram> builder = KeyBuilder<ShaderProgram>();
		
		for (const auto& [path, shaderType] : shaderPaths)
		{
			CORE_TRY(resolvedPath, ResolveFilePath(path, std::nullopt));
			builder.Add(resolvedPath.relative.generic_string());
			resolvedShaderPaths.emplace_back(std::move(resolvedPath), shaderType);
		}
		
		Key<ShaderProgram> key = builder.Finish("compiled_shader_program");
		if (auto cached = m_Storage.GetHandleByKey<ShaderProgram>(key); cached)
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
		return m_Storage.Emplace(key, std::move(programAsset)).value();
	}

	std::expected<Utils::Guid, Utils::Error> Manager::MakeMeshBufferId(std::string_view path, const std::filesystem::path& parentPath, const Import::BufferAccessor& accessor, const Import::ScenePrefab& source)
	{
		assert(accessor.buffer < source.buffers.size() && "Buffer accessor view references a buffer index that is out of bounds");
		const Import::Buffer& buffer = source.buffers[accessor.buffer];
		Utils::Hasher hasher = Utils::Hasher();
		
		hasher.Add(buffer.byteLength);
		CORE_TRY_DISCARD(std::visit(Utils::Types::Overloaded{
			[&hasher, parentPath, this](const Import::FileBuffer& fileBuffer) -> std::expected<void, Utils::Error>
			{
				CORE_TRY_CONTEXT(resolvedPath, this->ResolveFilePath(fileBuffer.file, parentPath), "Failed to resolve buffer file path: {}", fileBuffer.file.generic_string());
				hasher.Add(resolvedPath.relative.generic_string());
				hasher.Add(fileBuffer.byteOffset);
				return {};
			},
			[&hasher, path, bufferIndex = accessor.buffer](const Import::MemoryBuffer&) -> std::expected<void, Utils::Error>
			{
				hasher.Add(path);
				hasher.Add(bufferIndex);
				return {};
			}
		}, buffer.source));
		

		return hasher.Digest();
	}
	
	void Manager::AddBufferViewToMeshKey(KeyBuilder<Mesh>& builder, const Import::BufferAccessor& accessor)
	{
		builder.Add(accessor.view.byteOffset);
		builder.Add(accessor.view.byteStride);
		builder.Add(accessor.view.count);
		builder.Add(accessor.view.elementType);
		builder.Add(accessor.view.componentType);
		builder.Add(accessor.view.normalized);
	}
	
	std::expected<MeshCacheAccessors, Utils::Error> Manager::MakeMeshCacheAccessors(std::string_view path, const std::filesystem::path& parentPath, const Import::MeshAccessor& accessor, const Import::ScenePrefab& source)
	{
		if (accessor.topology != Import::PrimitiveTopology::Triangles)
			return std::unexpected(Utils::Error("Only triangle mesh topology is supported"));

		if (accessor.indexCount % 3 != 0)
			return std::unexpected(Utils::Error("Triangle mesh index count must be divisible by 3"));
		
		KeyBuilder<Mesh> builder = KeyBuilder<Mesh>();
		builder.Add(accessor.vertexCount);
		builder.Add(accessor.indexCount);
		builder.Add(static_cast<uint32_t>(accessor.topology));
		
		std::optional<Utils::Guid> indicesId;
		if (accessor.indices)
		{
			Import::BufferAccessor indices = accessor.indices.value();
			if (indices.view.elementType != Import::ElementType::Scalar)
				return std::unexpected(Utils::Error("Mesh indices must be scalar"));
			if (indices.view.componentType != Import::ComponentType::UnsignedByte 
				&& indices.view.componentType != Import::ComponentType::UnsignedShort 
				&& indices.view.componentType != Import::ComponentType::UnsignedInt)
				return std::unexpected(Utils::Error("Mesh indices must be unsigned integer type"));
			CORE_TRY_ASSIGN_CONTEXT(indicesId, MakeMeshBufferId(path, parentPath, indices, source), "Failed to make mesh buffer ID for indices");
			builder.Add(indicesId.value());
			AddBufferViewToMeshKey(builder, indices);
		}
		else
		{
			builder.Add("generated_indices");
		}

		if (!accessor.position)
			return std::unexpected(Utils::Error("Mesh must have positions"));
		Import::BufferAccessor positions = accessor.position.value();
		if (positions.view.elementType != Import::ElementType::Vec3)
			return std::unexpected(Utils::Error("Mesh positions must be 3D vectors"));
		if (positions.view.componentType != Import::ComponentType::Float)
			return std::unexpected(Utils::Error("Mesh positions must be floating-point numbers"));
		CORE_TRY_CONTEXT(positionsId, MakeMeshBufferId(path, parentPath, positions, source), "Failed to make mesh buffer ID for positions");
		builder.Add(positionsId);
		AddBufferViewToMeshKey(builder, positions);

		std::optional<Utils::Guid> normalsId;
		if (accessor.normal)
		{
			Import::BufferAccessor normals = accessor.normal.value();
			if (normals.view.elementType != Import::ElementType::Vec3)
				return std::unexpected(Utils::Error("Mesh normals must be 3D vectors"));
			if (normals.view.componentType != Import::ComponentType::Float)
				return std::unexpected(Utils::Error("Mesh normals must be floating-point numbers"));
			CORE_TRY_ASSIGN_CONTEXT(normalsId, MakeMeshBufferId(path, parentPath, normals, source), "Failed to make mesh buffer ID for normals");
			builder.Add(normalsId.value());
			AddBufferViewToMeshKey(builder, normals);
		}
		else
		{
			builder.Add("generated_normals");
		}

		std::optional<Utils::Guid> tangentsId;
		if (accessor.tangent)
		{
			Import::BufferAccessor tangents = accessor.tangent.value();
			if (tangents.view.elementType != Import::ElementType::Vec4)
				return std::unexpected(Utils::Error("Mesh tangents must be 4D vectors"));
			if (tangents.view.componentType != Import::ComponentType::Float)
				return std::unexpected(Utils::Error("Mesh tangents must be floating-point numbers"));
			CORE_TRY_ASSIGN_CONTEXT(tangentsId, MakeMeshBufferId(path, parentPath, tangents, source), "Failed to make mesh buffer ID for tangents");
			builder.Add(tangentsId.value());
			AddBufferViewToMeshKey(builder, tangents);
		}
		else
		{
			builder.Add("generated_tangents");
		}

		std::optional<Utils::Guid> uvsId;
		if (accessor.uv)
		{
			Import::BufferAccessor uvs = accessor.uv.value();
			if (uvs.view.elementType != Import::ElementType::Vec2)
				return std::unexpected(Utils::Error("Mesh UVs must be 2D vectors"));
			if (uvs.view.componentType != Import::ComponentType::Float)
				return std::unexpected(Utils::Error("Mesh UVs must be floating-point numbers"));
			CORE_TRY_ASSIGN_CONTEXT(uvsId, MakeMeshBufferId(path, parentPath, uvs, source), "Failed to make mesh buffer ID for UVs");
			builder.Add(uvsId.value());
			AddBufferViewToMeshKey(builder, uvs);
		}
		else
		{
			builder.Add("generated_uvs");
		}

		return MeshCacheAccessors {
			.mesh = builder.Finish("composed_mesh_primitive"),
			.indicesId = indicesId,
			.positionsId = positionsId,
			.normalsId = normalsId,
			.tangentsId = tangentsId,
			.uvsId = uvsId
		};
	}
	
	std::expected<void, Utils::Error> Manager::ImportMeshBuffer(std::span<uint8_t> destinationBufferBytes, const Import::BufferView& destinationView, const Import::BufferAccessor& accessor, const Import::ScenePrefab& source, Utils::Guid bufferId)
	{
		std::span<uint8_t> sourceBufferBytes;
		if (m_BufferCache.contains(bufferId))
		{
			sourceBufferBytes = std::span<uint8_t>{ m_BufferCache.at(bufferId) };
		}
		else
		{
			const Import::Buffer& bufferSource = source.buffers[accessor.buffer];
			
			CORE_TRY(loadedBuffer, std::visit(Utils::Types::Overloaded{
				[byteLength = bufferSource.byteLength, this](const Import::FileBuffer& fileBuffer) -> std::expected<std::vector<uint8_t>, Utils::Error>
				{
					CORE_TRY_CONTEXT(resolvedPath, this->ResolveFilePath(fileBuffer.file, std::nullopt), "Failed to resolve buffer file path: {}", fileBuffer.file.generic_string());
					CORE_TRY_CONTEXT(buffer, Import::ReadFileBytes(resolvedPath.absolute, byteLength, fileBuffer.byteOffset), "Failed to read buffer from file: {}", resolvedPath.absolute.generic_string());
					return buffer;
				},
				[](const Import::MemoryBuffer& memoryBuffer) -> std::expected<std::vector<uint8_t>, Utils::Error>
				{
					return memoryBuffer.bytes;
				}
			}, source.buffers[accessor.buffer].source));

			auto [iterator, inserted] = m_BufferCache.emplace(bufferId, std::move(loadedBuffer));
			sourceBufferBytes = std::span<uint8_t>{iterator->second};
		}
		
		CORE_TRY_DISCARD_CONTEXT(Import::CopyViewData(destinationBufferBytes, destinationView, sourceBufferBytes, accessor.view), "Failed to copy view data");
		return {};
	}

	std::expected<Import::MeshPrimitive, Utils::Error> Manager::ComposeMesh(const Import::MeshAccessor& accessor, const Import::ScenePrefab& source, const MeshCacheAccessors& cacheAccessors)
	{
		Import::MeshPrimitive mesh;
		std::span<uint8_t> meshBufferBytes = std::span<uint8_t>{ reinterpret_cast<uint8_t*>(mesh.vertices.data()), mesh.vertices.size() * sizeof(Graphics::Vertex) };

		if (accessor.indices)
		{
			std::span<uint8_t> indexBufferBytes = std::span<uint8_t>{ reinterpret_cast<uint8_t*>(mesh.indices.data()), mesh.indices.size() * sizeof(uint32_t) };
			CORE_TRY_DISCARD_CONTEXT(
				ImportMeshBuffer(
					indexBufferBytes, 
					Import::IndexView(static_cast<uint32_t>(mesh.indices.size())), 
					accessor.indices.value(), 
					source, 
					cacheAccessors.indicesId.value()), 
				"Failed to import index buffer");
		}
		else
		{
			spdlog::warn("Mesh {} has no indices, generating default index buffer", accessor.index);
			mesh.indices = std::views::iota(std::uint32_t{0}, static_cast<std::uint32_t>(mesh.vertices.size())) | std::ranges::to<std::vector<std::uint32_t>>();
		}
		
		assert(accessor.position && "Mesh must have positions");
		CORE_TRY_DISCARD_CONTEXT(
			ImportMeshBuffer(
				meshBufferBytes, 
				Import::PositionView(static_cast<uint32_t>(mesh.vertices.size())), 
				accessor.position.value(), 
				source, 
				cacheAccessors.positionsId), 
			"Failed to import position buffer");

		if (accessor.normal)
		{
			CORE_TRY_DISCARD_CONTEXT(
				ImportMeshBuffer(
					meshBufferBytes, 
					Import::NormalView(static_cast<uint32_t>(mesh.vertices.size())), 
					accessor.normal.value(), 
					source, 
					cacheAccessors.normalsId.value()), 
				"Failed to import normal buffer");
		}
		else
		{
			spdlog::warn("Mesh {} has no normals, generating geometric normals based on positions", accessor.index);
			return std::unexpected(Utils::Error("Generation of geometric normals is not implemented yet"));
		}

		if (accessor.tangent)
		{
			CORE_TRY_DISCARD_CONTEXT(
				ImportMeshBuffer(
					meshBufferBytes, 
					Import::TangentView(static_cast<uint32_t>(mesh.vertices.size())), 
					accessor.tangent.value(), 
					source, 
					cacheAccessors.tangentsId.value()), 
				"Failed to import tangent buffer");
		}
		else
		{
			spdlog::info("Mesh {} has no dedicated tangent buffer, adding generated tangents", accessor.index);
			Utils::Math::GenerateTangentSpace(mesh.vertices, mesh.indices);
		}

		if (accessor.uv)
		{
			CORE_TRY_DISCARD_CONTEXT(
				ImportMeshBuffer(
					meshBufferBytes, 
					Import::UVView(static_cast<uint32_t>(mesh.vertices.size())), 
					accessor.uv.value(), 
					source, 
					cacheAccessors.uvsId.value()), 
				"Failed to import UV buffer");
		}
		else
		{
			spdlog::warn("Mesh {} has no UVs, generating default UV buffer", accessor.index);
			return std::unexpected(Utils::Error("UV generation is not implemented yet"));
		}

		return mesh;
	}

	std::expected<Handle<Mesh>, Utils::Error> Manager::ImportMesh(const Utils::Path::ResolvedPath& path, const std::filesystem::path& parentPath, const Import::ScenePrefab& prefab, Import::MeshAccessor mesh, uint32_t index)
	{
		std::string relativeStr = path.relative.generic_string();
		CORE_TRY_CONTEXT(cacheAccessors, MakeMeshCacheAccessors(relativeStr, parentPath, mesh, prefab), "Failed to make mesh key for mesh {}, file path: {}", index, path.absolute.generic_string());

		if (auto cached = m_Storage.GetHandleByKey<Mesh>(cacheAccessors.mesh); cached)
			return cached.value();
		
		CORE_TRY_CONTEXT(meshPrimitive, ComposeMesh(mesh, prefab, cacheAccessors), "Failed to compose mesh {}", mesh.index);

		CORE_TRY_CONTEXT(glMesh, Graphics::Gl::Mesh::Create(meshPrimitive), "Failed to create GL mesh");
		Graphics::Cpu::Mesh cpuMesh = Graphics::Cpu::Mesh::Create(std::move(meshPrimitive));

		Mesh asset(std::move(cpuMesh), std::move(glMesh));
		return m_Storage.Emplace(cacheAccessors.mesh, std::move(asset));
	}
	
	std::expected<Handle<Texture>, Utils::Error> Manager::ImportFileOrPixelTexture(
		const std::optional<Utils::Path::ResolvedPath>& path,	
		const Graphics::PixelFormat& defaultFormat, 
		std::span<const uint8_t> defaultValue,
		bool flipVertically)
	{
		if (path)
			return ImportTexture(path.value(), defaultFormat.colorSpace, flipVertically);

		return ImportPixelTexture(defaultFormat, defaultValue);
	}

	std::expected<Handle<Texture>, Utils::Error> Manager::ImportOrmTexture(const ResolvedMaterial& resolvedMaterial)
	{
		Key<Texture> key = MakeOrmTextureKey(resolvedMaterial);
		if (auto cached = m_Storage.GetHandleByKey<Texture>(key); cached)
			return cached.value();
		
		CORE_TRY_CONTEXT(image, CreateOrmTextureImage(resolvedMaterial), "Failed to create ORM texture image");

		CORE_TRY_CONTEXT(glTexture, Graphics::Gl::Texture::Create2D(image), "Failed to create GL texture for ORM texture");
		CORE_TRY_CONTEXT(cudaTexture, Graphics::Cuda::Texture::Create2D(Import::Media::ExtractRmTexture(image)), "Failed to create CUDA texture for ORM texture");
		Graphics::Cpu::Texture cpuTexture = Graphics::Cpu::Texture::Create(std::move(image));

		Texture asset(std::move(cpuTexture), std::move(glTexture), std::move(cudaTexture));
		return m_Storage.Emplace(key, std::move(asset));
	}
	
	std::expected<Handle<Material>, Utils::Error> Manager::ImportMaterial(const Utils::Path::ResolvedPath& path, const std::filesystem::path& parentPath, const Import::Material& materialSource, uint32_t index)
	{
		std::string relativeStr = path.relative.generic_string();

		KeyBuilder<Material> builder = KeyBuilder<Material>();
		builder.Add(relativeStr);
		builder.Add(index);
		Key<Material> key = builder.Finish("imported_material");

		if (auto cached = m_Storage.GetHandleByKey<Material>(key); cached)
			return cached.value();

		CORE_TRY_CONTEXT(resolvedMaterial, ResolveMaterial(path, parentPath, materialSource), "Failed to resolve material");

		std::array<uint8_t, 3> colorPixel = LinearRgbToSrgbU8(resolvedMaterial.color);
		std::array<uint8_t, 3> specularPixel = LinearRgbToSrgbU8(resolvedMaterial.specular);
		std::array<uint8_t, 1> shininessPixel = 
		{ 
			Utils::NumberTypes::NormalizedFloatToU8(Utils::NumberTypes::NormalizeFloat(resolvedMaterial.shininess, Graphics::MaterialDefaults::MinShininess, Graphics::MaterialDefaults::MaxShininess))
		};

		std::span<const uint8_t> emissionPixel = 
		{
			reinterpret_cast<const uint8_t*>(&resolvedMaterial.emission[0]),
			sizeof(resolvedMaterial.emission)
		};

		bool flipTextures = resolvedMaterial.flipTextures;
		

		CORE_TRY_CONTEXT(
			color, 
			ImportFileOrPixelTexture(resolvedMaterial.colorTexture, {Graphics::ChannelLayout::RGB, Graphics::ComponentType::UInt8, Graphics::ColorSpace::SRGB}, colorPixel, flipTextures),
			"Failed to import albedo texture");
		CORE_TRY_CONTEXT(
			specular, 
			ImportFileOrPixelTexture(resolvedMaterial.specularTexture, {Graphics::ChannelLayout::RGB, Graphics::ComponentType::UInt8, Graphics::ColorSpace::SRGB}, specularPixel, flipTextures),
			"Failed to import specular texture");
		CORE_TRY_CONTEXT(
			shininess, 
			ImportFileOrPixelTexture(resolvedMaterial.shininessTexture, {Graphics::ChannelLayout::R, Graphics::ComponentType::UInt8, Graphics::ColorSpace::Linear}, shininessPixel, flipTextures), 
			"Failed to import shininess texture");
		CORE_TRY_CONTEXT(
			orm, 
			ImportOrmTexture(resolvedMaterial),
			"Failed to import ORM texture");
		CORE_TRY_CONTEXT(
			emission, 
			ImportFileOrPixelTexture(resolvedMaterial.emissionTexture, {Graphics::ChannelLayout::RGB, Graphics::ComponentType::Float32, Graphics::ColorSpace::Linear}, emissionPixel, flipTextures), 
			"Failed to import emission texture");
		CORE_TRY_CONTEXT(
			normal, 
			ImportFileOrPixelTexture(resolvedMaterial.normalTexture, {Graphics::ChannelLayout::RGB, Graphics::ComponentType::UInt8, Graphics::ColorSpace::Linear}, Graphics::MaterialDefaults::DefaultNormal, flipTextures), 
			"Failed to import normal texture");

		Material asset(
			resolvedMaterial.surface,
			color,
			specular,
			shininess,
			orm,
			emission,
			normal,
			resolvedMaterial.ior,
			resolvedMaterial.transmission,
			resolvedMaterial.opacity);

		return m_Storage.Emplace(key, std::move(asset));
	}

	std::expected<Handle<Material>, Utils::Error> Manager::ImportDefaultMaterial()
	{
		Key<Material> key = MakePathSourcedKey<Material>(DefaultMaterialSource, "default_material");

		if (auto cached = m_Storage.GetHandleByKey<Material>(key); cached)
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

		
		std::array<uint8_t, 3> ormPixel = {
			Utils::NumberTypes::NormalizedFloatToU8(Graphics::MaterialDefaults::Occlusion),
			Utils::NumberTypes::NormalizedFloatToU8(Graphics::MaterialDefaults::Roughness),
			Utils::NumberTypes::NormalizedFloatToU8(Graphics::MaterialDefaults::Metallic),
		};

		CORE_TRY_CONTEXT(color, ImportPixelTexture(rgbSrgb, Graphics::MaterialDefaults::DefaultColor), "Failed to import default albedo texture");
		CORE_TRY_CONTEXT(specular, ImportPixelTexture(rgbSrgb, Graphics::MaterialDefaults::DefaultPhongSpecular), "Failed to import default specular texture");
		CORE_TRY_CONTEXT(shininess, ImportPixelTexture(rLinear, Graphics::MaterialDefaults::DefaultShininess), "Failed to import default shininess texture");
		CORE_TRY_CONTEXT(orm, ImportPixelTexture(rgbLinear, ormPixel), "Failed to import default RMA texture");
		
		const std::array<float, 3>& emissionDefault = Graphics::MaterialDefaults::DefaultEmission;
		CORE_TRY_CONTEXT(emission, ImportPixelTexture(rgbLinearHdr, { reinterpret_cast<const uint8_t*>(&emissionDefault[0]), sizeof(emissionDefault) }), "Failed to import default emission texture");
		CORE_TRY_CONTEXT(normal, ImportPixelTexture(rgbLinear, Graphics::MaterialDefaults::DefaultNormal), "Failed to import default normal texture");

		Material asset(
			Graphics::MaterialDefaults::DefaultSurfaceModel,
			color,
			specular,
			shininess,
			orm,
			emission,
			normal,
			Graphics::MaterialDefaults::DefaultIor,
			Graphics::MaterialDefaults::DefaultTransmission,
			Graphics::MaterialDefaults::DefaultOpacity);

		return m_Storage.Emplace(key, std::move(asset));
	}


	std::expected<Handle<Texture>, Utils::Error> Manager::ImportPixelTexture(const Graphics::PixelFormat& format, std::span<const uint8_t> value)
	{
		KeyBuilder<Texture> builder = KeyBuilder<Texture>();
		builder.Add(format.layout);
		builder.Add(format.componentType);
		builder.Add(format.colorSpace);
		uint64_t length = value.size();
		builder.Add(length);
		builder.Add(value);
		Key<Texture> key = builder.Finish("pixel_texture");
		
		if (auto cached = m_Storage.GetHandleByKey<Texture>(key); cached)
			return cached.value();

		Import::Media::Image image{
			.width = 1,
			.height = 1,
			.format = format,
			.data = { value.begin(), value.end() }
		};

		CORE_TRY(glTexture, Graphics::Gl::Texture::Create2D(image));
		CORE_TRY(cudaTexture, Graphics::Cuda::Texture::Create2D(image));

		Texture asset(
			Graphics::Cpu::Texture::Create(std::move(image)),
			std::move(glTexture),
			std::move(cudaTexture));

		return m_Storage.Emplace(key, std::move(asset));
	}
	

	std::expected<std::optional<Utils::Path::ResolvedPath>, Utils::Error> Manager::ResolveTexturePath(
		const std::optional<std::filesystem::path>& texturePath,
		const std::filesystem::path& root)
	{
		if (!texturePath)
			return std::nullopt;

		CORE_TRY_CONTEXT(resolved, ResolveFilePath(texturePath.value(), root), "Failed to resolve texture path: {}", texturePath.value().string());
		return resolved;
	}
	
	std::expected<ResolvedMaterial, Utils::Error> Manager::ResolveMaterial(const Utils::Path::ResolvedPath& path, const std::filesystem::path& parentPath, const Import::Material& materialSource)
	{
		Graphics::SurfaceModel surface = ResolveSurfaceModel(materialSource);

		auto localShading = Graphics::Gl::ToLocalShadingChecked(surface);
		if (!localShading.second)
			spdlog::warn(
				"Material '{}' uses surface model, which is not supported with local illumination."
				"It will be treated as Unlit when shading via OpenGL. File path: {}",
				materialSource.name,
				path.absolute.generic_string());
		
		auto globalShading = Graphics::Cuda::ToGlobalShadingChecked(surface);
		if (!globalShading.second)
			spdlog::warn(
				"Material '{}' uses surface model, which is not supported with global illumination."
				"It will be treated as Diffuse when shading via CUDA. File path: {}",
				materialSource.name,
				path.absolute.generic_string());

		bool flipTextures = materialSource.flipTextures.value_or(false);
		
		ResolvedMaterial resolved;

		resolved.surface = surface;
		resolved.flipTextures = flipTextures;
		resolved.color = materialSource.color;
		resolved.specular = materialSource.specular;
		resolved.shininess = materialSource.shininess;
		resolved.roughness = materialSource.roughness;
		resolved.metallic = materialSource.metallic;
		resolved.emission = materialSource.emission;
		resolved.ior = materialSource.ior;
		resolved.transmission = materialSource.transmission;
		resolved.opacity = materialSource.opacity;
		
		CORE_TRY_ASSIGN(resolved.colorTexture, ResolveTexturePath(materialSource.colorTexture, parentPath));
		CORE_TRY_ASSIGN(resolved.specularTexture, ResolveTexturePath(materialSource.specularTexture, parentPath));
		CORE_TRY_ASSIGN(resolved.shininessTexture, ResolveTexturePath(materialSource.shininessTexture, parentPath));
		CORE_TRY_ASSIGN(resolved.occlusionTexture, ResolveTexturePath(materialSource.occlusionTexture, parentPath));
		CORE_TRY_ASSIGN(resolved.roughnessTexture, ResolveTexturePath(materialSource.roughnessTexture, parentPath));
		CORE_TRY_ASSIGN(resolved.metallicTexture, ResolveTexturePath(materialSource.metallicTexture, parentPath));
		CORE_TRY_ASSIGN(resolved.rmTexture, ResolveTexturePath(materialSource.rmTexture, parentPath));
		CORE_TRY_ASSIGN(resolved.ormTexture, ResolveTexturePath(materialSource.ormTexture, parentPath));
		CORE_TRY_ASSIGN(resolved.rmoTexture, ResolveTexturePath(materialSource.rmoTexture, parentPath));
		CORE_TRY_ASSIGN(resolved.emissionTexture, ResolveTexturePath(materialSource.emissionTexture, parentPath));
		CORE_TRY_ASSIGN(resolved.normalTexture, ResolveTexturePath(materialSource.normalTexture, parentPath));

		return resolved;
	}
	
	std::expected<Utils::Path::ResolvedPath, Utils::Error> Manager::ResolveFilePath(const std::filesystem::path& path, const std::optional<std::filesystem::path>& root)
	{
		Utils::Path::ResolvedPath resolvedPath;

		if (root)
		{
			CORE_TRY_ASSIGN(resolvedPath, Utils::Path::ResolvePath(path, root.value()));
			
			if (!std::filesystem::exists(resolvedPath.absolute))
			{
				CORE_TRY_ASSIGN(resolvedPath, Utils::Path::ResolvePath(path, m_Root));
				if (!std::filesystem::exists(resolvedPath.absolute))
					return std::unexpected(Utils::Error("Resolved file path does not exist: {}", resolvedPath.absolute.generic_string()));
			}
		}
		else
		{
			CORE_TRY_ASSIGN(resolvedPath, Utils::Path::ResolvePath(path, m_Root));
		}
		
		if (!Utils::Path::IsSubpath(m_Root, resolvedPath.absolute))
			return std::unexpected(Utils::Error("Resolved file path is not a subpath of the asset root: {}", resolvedPath.absolute.generic_string()));

		if (root)
			resolvedPath.relative = std::filesystem::relative(resolvedPath.absolute, m_Root);

		return resolvedPath;
	}
}
