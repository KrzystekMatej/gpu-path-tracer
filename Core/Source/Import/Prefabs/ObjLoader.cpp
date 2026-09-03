#include <Core/Import/Prefabs/ObjLoader.hpp>

#include <array>
#include <cstddef>
#include <cstring>
#include <format>
#include <map>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <tiny_obj_loader.h>
#include <spdlog/spdlog.h>
#include <glm/geometric.hpp>

#include <Core/Utils/Math/TangentSpace.hpp>
#include <Core/Import/Prefabs/Extras.hpp>
#include <Core/Graphics/Common/Vertex.hpp>

namespace Core::Import
{
	namespace
	{
		struct ObjIndexKey
		{
			int32_t position;
			int32_t texcoord;
			int32_t normal;

			friend bool operator==(const ObjIndexKey& first, const ObjIndexKey& second) noexcept
			{
				return first.position == second.position &&
					first.texcoord == second.texcoord &&
					first.normal == second.normal;
			}
		};

		struct ObjIndexKeyHash
		{
			static size_t Combine(size_t hash, uint32_t value) noexcept
			{
				return hash ^ (static_cast<size_t>(value) + 0x9e3779b97f4a7c15ull + (hash << 6) + (hash >> 2));
			}

			size_t operator()(const ObjIndexKey& key) const noexcept
			{
				size_t hash = 0;
				hash = Combine(hash, static_cast<uint32_t>(key.position));
				hash = Combine(hash, static_cast<uint32_t>(key.texcoord));
				hash = Combine(hash, static_cast<uint32_t>(key.normal));
				return hash;
			}
		};

		struct SubmeshBuilder
		{
			std::vector<Graphics::Vertex> vertices;
			std::vector<uint32_t> indices;
			std::unordered_map<ObjIndexKey, uint32_t, ObjIndexKeyHash> indexMap;
			bool hasPositions = false;
			bool hasNormals = false;
			bool hasTangents = false;
			bool hasUvs = false;
		};

		const std::string RmTextureKey = "map_RM";
		const std::string RmoTextureKey = "map_RMO";
		const std::string OrmTextureKey = "map_ORM";

		template<typename T>
		uint32_t PushMemoryBuffer(ScenePrefab& prefab, const std::vector<T>& values)
		{
			Buffer buffer{};
			buffer.byteLength = static_cast<uint64_t>(values.size() * sizeof(T));

			MemoryBuffer memory{};
			memory.bytes.resize(static_cast<size_t>(buffer.byteLength));

			if (!memory.bytes.empty())
				std::memcpy(memory.bytes.data(), values.data(), memory.bytes.size());

			buffer.source = std::move(memory);

			uint32_t bufferIndex = static_cast<uint32_t>(prefab.buffers.size());
			prefab.buffers.push_back(std::move(buffer));

			return bufferIndex;
		}

		MeshAccessor CreateMeshAccessor(ScenePrefab& prefab, const SubmeshBuilder& builder, std::optional<uint32_t> material)
		{
			uint32_t vertexBuffer = PushMemoryBuffer(prefab, builder.vertices);
			uint32_t indexBuffer = PushMemoryBuffer(prefab, builder.indices);
			uint32_t primitiveIndex = static_cast<uint32_t>(prefab.meshPrimitivesAccessors.size());

			return MeshAccessor
			{
				.index = primitiveIndex,
				.vertexCount = static_cast<uint32_t>(builder.vertices.size()),
				.indexCount = static_cast<uint32_t>(builder.indices.size()),
				.topology = PrimitiveTopology::Triangles,
				.material = material,
				.indices = BufferAccessor {.buffer = indexBuffer, .view = IndexView(builder.indices.size())},
				.position = builder.hasPositions ? std::optional<BufferAccessor>(BufferAccessor {.buffer = vertexBuffer, .view = PositionView(builder.vertices.size())}) : std::nullopt,
				.normal = builder.hasNormals ? std::optional<BufferAccessor>(BufferAccessor {.buffer = vertexBuffer, .view = NormalView(builder.vertices.size())}) : std::nullopt,
				.tangent = builder.hasTangents ? std::optional<BufferAccessor>(BufferAccessor {.buffer = vertexBuffer, .view = TangentView(builder.vertices.size())}) : std::nullopt,
				.uv = builder.hasUvs ? std::optional<BufferAccessor>(BufferAccessor {.buffer = vertexBuffer, .view = UVView(builder.vertices.size())}) : std::nullopt
			};
		}

		std::optional<Graphics::SurfaceModel> ReadSurfaceModel(const tinyobj::material_t& source, const std::filesystem::path& path)
		{
			std::map<std::string, std::string>::const_iterator iterator = source.unknown_parameter.find(SurfaceModelKey);

			if (iterator == source.unknown_parameter.end())
				return std::nullopt;

			return ParseSurfaceModel(iterator->second);
		}

		std::vector<Material> ConvertMaterials(const std::vector<tinyobj::material_t>& materialSources, const std::filesystem::path& path)
		{
			std::vector<Material> materials;
			materials.reserve(materialSources.size());

			for (const tinyobj::material_t& source : materialSources)
			{
				std::optional<Graphics::SurfaceModel> surface = ReadSurfaceModel(source, path);

				std::optional<std::string> normal = source.normal_texname.empty() ? std::nullopt : std::make_optional(source.normal_texname);
				std::optional<std::string> bump = source.bump_texname.empty() ? std::nullopt : std::make_optional(source.bump_texname);
				auto rmIterator = source.unknown_parameter.find(RmTextureKey);
				auto rmoIterator = source.unknown_parameter.find(RmoTextureKey);
				auto ormIterator = source.unknown_parameter.find(OrmTextureKey);
				auto flipTexturesIterator = source.unknown_parameter.find(FlipTexturesKey);

				materials.push_back(Material
				{
					.name = source.name,
					.surface = surface,
					.flipTextures = (flipTexturesIterator != source.unknown_parameter.end() && flipTexturesIterator->second == "true")
						? std::make_optional(true) : std::nullopt,
					.color = { source.diffuse[0], source.diffuse[1], source.diffuse[2] },
					.specular = { source.specular[0], source.specular[1], source.specular[2] },
					.shininess = source.shininess,
					.roughness = source.roughness,
					.metallic = source.metallic,
					.emission = { source.emission[0], source.emission[1], source.emission[2] },
					.ior = source.ior,
					.transmission = source.transmittance[0],
					.opacity = source.dissolve,
					.colorTexture = source.diffuse_texname.empty() ? std::nullopt : std::make_optional(source.diffuse_texname),
					.specularTexture = source.specular_texname.empty() ? std::nullopt : std::make_optional(source.specular_texname),
					.shininessTexture = source.specular_highlight_texname.empty() ? std::nullopt : std::make_optional(source.specular_highlight_texname),
					.occlusionTexture = source.ambient_texname.empty() ? std::nullopt : std::make_optional(source.ambient_texname),
					.roughnessTexture = source.roughness_texname.empty() ? std::nullopt : std::make_optional(source.roughness_texname),
					.metallicTexture = source.metallic_texname.empty() ? std::nullopt : std::make_optional(source.metallic_texname),
					.rmTexture = rmIterator == source.unknown_parameter.end() || rmIterator->second.empty() ? std::nullopt : std::make_optional(rmIterator->second),
					.ormTexture = ormIterator == source.unknown_parameter.end() || ormIterator->second.empty() ? std::nullopt : std::make_optional(ormIterator->second),
					.rmoTexture = rmoIterator == source.unknown_parameter.end() || rmoIterator->second.empty() ? std::nullopt : std::make_optional(rmoIterator->second),
					.emissionTexture = source.emissive_texname.empty() ? std::nullopt : std::make_optional(source.emissive_texname),
					.normalTexture = normal ? normal : bump,
				});
			}

			return materials;
		}

		std::optional<uint32_t> ConvertMaterialIndex(int materialId, const std::vector<tinyobj::material_t>& materialSources, const std::filesystem::path& path)
		{
			if (materialId < 0)
				return std::nullopt;

			if (static_cast<size_t>(materialId) >= materialSources.size())
			{
				spdlog::warn("OBJ material index {} is out of range. File path: {}", materialId, path.string());
				return std::nullopt;
			}

			return static_cast<uint32_t>(materialId);
		}
		
		std::expected<void, Utils::Error> ValidateObjIndex(
			const tinyobj::attrib_t& attrib,
			const tinyobj::index_t& index,
			const std::filesystem::path& path)
		{
			if (index.vertex_index < 0)
				return std::unexpected(Utils::Error("OBJ face vertex is missing a position index. File path: {}", path.string()));

			const size_t positionIndex = static_cast<size_t>(index.vertex_index);
			if (positionIndex >= attrib.vertices.size() / 3)
			{
				return std::unexpected(Utils::Error(
					"OBJ position index {} is out of bounds. Position float count: {}, file path: {}",
					index.vertex_index,
					attrib.vertices.size(),
					path.string()));
			}

			if (index.normal_index >= 0)
			{
				const size_t normalIndex = static_cast<size_t>(index.normal_index);
				if (normalIndex >= attrib.normals.size() / 3)
				{
					return std::unexpected(Utils::Error(
						"OBJ normal index {} is out of bounds. Normal float count: {}, file path: {}",
						index.normal_index,
						attrib.normals.size(),
						path.string()));
				}
			}

			if (index.texcoord_index >= 0)
			{
				const size_t texcoordIndex = static_cast<size_t>(index.texcoord_index);
				if (texcoordIndex >= attrib.texcoords.size() / 2)
				{
					return std::unexpected(Utils::Error(
						"OBJ texcoord index {} is out of bounds. Texcoord float count: {}, file path: {}",
						index.texcoord_index,
						attrib.texcoords.size(),
						path.string()));
				}
			}

			return {};
		}

		std::expected<void, Utils::Error> AppendFaceVertex(SubmeshBuilder& builder, const tinyobj::attrib_t& attrib, const tinyobj::index_t& index, const std::filesystem::path& path)
		{
			CORE_TRY_DISCARD(ValidateObjIndex(attrib, index, path));
			

			Graphics::Vertex vertex{};

			builder.hasPositions = true;
			vertex.position.x = attrib.vertices[3 * static_cast<size_t>(index.vertex_index) + 0];
			vertex.position.y = attrib.vertices[3 * static_cast<size_t>(index.vertex_index) + 1];
			vertex.position.z = attrib.vertices[3 * static_cast<size_t>(index.vertex_index) + 2];

			bool firstVertex = builder.vertices.empty();
			builder.hasNormals = index.normal_index >= 0 && (firstVertex || builder.hasNormals);
			if (builder.hasNormals)
			{
				vertex.normal.x = attrib.normals[3 * static_cast<size_t>(index.normal_index) + 0];
				vertex.normal.y = attrib.normals[3 * static_cast<size_t>(index.normal_index) + 1];
				vertex.normal.z = attrib.normals[3 * static_cast<size_t>(index.normal_index) + 2];
				vertex.normal = glm::normalize(vertex.normal);
			}
			
			builder.hasUvs = index.texcoord_index >= 0 && (firstVertex || builder.hasUvs);
			if (builder.hasUvs)
			{		
				vertex.uv.x = attrib.texcoords[2 * static_cast<size_t>(index.texcoord_index) + 0];
				vertex.uv.y = attrib.texcoords[2 * static_cast<size_t>(index.texcoord_index) + 1];
			}

			ObjIndexKey key
			{
				.position = static_cast<int32_t>(index.vertex_index),
				.texcoord = static_cast<int32_t>(index.texcoord_index),
				.normal = static_cast<int32_t>(index.normal_index)
			};

			auto result = builder.indexMap.try_emplace(key, static_cast<uint32_t>(builder.vertices.size()));

			if (result.second)
				builder.vertices.push_back(vertex);

			builder.indices.push_back(result.first->second);

			return {};
		}

		std::expected<Mesh, Utils::Error> ConvertShape(
			ScenePrefab& prefab,
			const tinyobj::attrib_t& attrib,
			const tinyobj::shape_t& shape,
			const std::vector<tinyobj::material_t>& materialSources,
			const std::filesystem::path& path)
		{
			constexpr size_t triangleVertexCount = 3;

			size_t indexOffset = 0;
			std::map<int, SubmeshBuilder> submeshes;

			for (size_t faceIndex = 0; faceIndex < shape.mesh.num_face_vertices.size(); faceIndex++)
			{
				size_t faceVertexCount = static_cast<size_t>(shape.mesh.num_face_vertices[faceIndex]);

				if (faceVertexCount != triangleVertexCount)
					return std::unexpected(Utils::Error("OBJ triangulation failed, face {} has {} vertices. File path: {}", faceIndex, faceVertexCount, path.string()));

				int materialId = shape.mesh.material_ids.empty() ? -1 : shape.mesh.material_ids[faceIndex];

				SubmeshBuilder& builder = submeshes[materialId];

				for (size_t faceVertexIndex = 0; faceVertexIndex < faceVertexCount; faceVertexIndex++)
				{
					const tinyobj::index_t& index = shape.mesh.indices[indexOffset + faceVertexIndex];
					CORE_TRY_DISCARD(AppendFaceVertex(builder, attrib, index, path));
				}

				indexOffset += faceVertexCount;
			}

			Mesh mesh{};
			mesh.primitives.reserve(submeshes.size());

			for (std::pair<const int, SubmeshBuilder>& submesh : submeshes)
			{
				int materialId = submesh.first;
				SubmeshBuilder& builder = submesh.second;

				std::optional<uint32_t> material = ConvertMaterialIndex(materialId, materialSources, path);

				MeshAccessor primitive = CreateMeshAccessor(prefab, builder, material);
				uint32_t primitiveIndex = primitive.index;
				prefab.meshPrimitivesAccessors.push_back(std::move(primitive));
				mesh.primitives.push_back(primitiveIndex);
			}

			return mesh;
		}
	}

	std::expected<ScenePrefab, Utils::Error> LoadObj(const std::filesystem::path& path)
	{
		tinyobj::ObjReaderConfig readerConfig;
		readerConfig.mtl_search_path = path.parent_path().string();
		readerConfig.triangulate = true;
		readerConfig.vertex_color = false;

		tinyobj::ObjReader reader;

		if (!reader.ParseFromFile(path.string(), readerConfig))
		{
			if (!reader.Error().empty())
				return std::unexpected(Utils::Error("TinyObjReader failed, error: {}", reader.Error()));

			return std::unexpected(Utils::Error("TinyObjReader failed, error: unknown, file path: {}", path.string()));
		}

		if (!reader.Warning().empty())
			spdlog::warn("TinyObjReader warning while loading '{}': {}", path.string(), reader.Warning());

		const tinyobj::attrib_t& attrib = reader.GetAttrib();
		const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();
		const std::vector<tinyobj::material_t>& materialSources = reader.GetMaterials();

		if (materialSources.empty())
			spdlog::warn("No materials were loaded for '{}'. Faces referencing materials will be ignored.", path.string());

		ScenePrefab prefab{};
		prefab.name = path.stem().string();
		prefab.materials = ConvertMaterials(materialSources, path);

		SceneNode root{};
		root.name = prefab.name;

		uint32_t rootIndex = static_cast<uint32_t>(prefab.nodes.size());
		prefab.nodes.push_back(std::move(root));
		prefab.roots.push_back(rootIndex);

		for (size_t shapeIndex = 0; shapeIndex < shapes.size(); shapeIndex++)
		{
			const tinyobj::shape_t& shape = shapes[shapeIndex];

			CORE_TRY_CONTEXT(mesh, ConvertShape(prefab, attrib, shape, materialSources, path), "Failed to import OBJ shape {}", shapeIndex);

			if (mesh.primitives.empty())
				continue;

			uint32_t meshIndex = static_cast<uint32_t>(prefab.meshes.size());
			prefab.meshes.push_back(std::move(mesh));

			SceneNode node{};
			node.name = shape.name.empty() ? std::format("{}_{}", prefab.name, shapeIndex) : shape.name;
			node.mesh = meshIndex;

			uint32_t nodeIndex = static_cast<uint32_t>(prefab.nodes.size());
			prefab.nodes.push_back(std::move(node));
			prefab.nodes[rootIndex].children.push_back(nodeIndex);
		}

		return prefab;
	}
}