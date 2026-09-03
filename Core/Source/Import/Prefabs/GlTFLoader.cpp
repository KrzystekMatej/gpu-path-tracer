#include <Core/Import/Prefabs/GlTFLoader.hpp>

#include <unordered_map>
#include <cstring>
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <simdjson.h>

#include <Core/Utils/Error.hpp>
#include <Core/Import/Prefabs/Extras.hpp>
#include <Core/Graphics/Common/Material.hpp>

namespace Core::Import
{
	namespace
	{
		using MeshMap = std::unordered_map<size_t, uint32_t>;
		using NodeMap = std::unordered_map<size_t, uint32_t>;
		
		struct MaterialExtras
		{
			std::optional<Graphics::SurfaceModel> surface;
			std::optional<bool> flipTextures;
		};

		std::expected<ElementType, Utils::Error> ConvertElementType(fastgltf::AccessorType type)
		{
			switch (type)
			{
			case fastgltf::AccessorType::Scalar:
				return ElementType::Scalar;
			case fastgltf::AccessorType::Vec2:
				return ElementType::Vec2;
			case fastgltf::AccessorType::Vec3:
				return ElementType::Vec3;
			case fastgltf::AccessorType::Vec4:
				return ElementType::Vec4;
			case fastgltf::AccessorType::Mat2:
				return ElementType::Mat2;
			case fastgltf::AccessorType::Mat3:
				return ElementType::Mat3;
			case fastgltf::AccessorType::Mat4:
				return ElementType::Mat4;
			default:
				return std::unexpected(Utils::Error("Unsupported glTF accessor element type"));
			}
		}

		std::expected<ComponentType, Utils::Error> ConvertComponentType(fastgltf::ComponentType type)
		{
			switch (type)
			{
			case fastgltf::ComponentType::Byte:
				return ComponentType::Byte;
			case fastgltf::ComponentType::UnsignedByte:
				return ComponentType::UnsignedByte;
			case fastgltf::ComponentType::Short:
				return ComponentType::Short;
			case fastgltf::ComponentType::UnsignedShort:
				return ComponentType::UnsignedShort;
			case fastgltf::ComponentType::Int:
				return ComponentType::Int;
			case fastgltf::ComponentType::UnsignedInt:
				return ComponentType::UnsignedInt;
			case fastgltf::ComponentType::Float:
				return ComponentType::Float;
			case fastgltf::ComponentType::Double:
				return ComponentType::Double;
			default:
				return std::unexpected(Utils::Error("Unsupported glTF accessor component type"));
			}
		}

		std::expected<PrimitiveTopology, Utils::Error> ConvertTopology(fastgltf::PrimitiveType type)
		{
			switch (type)
			{
			case fastgltf::PrimitiveType::Points:
				return PrimitiveTopology::Points;
			case fastgltf::PrimitiveType::Lines:
				return PrimitiveTopology::Lines;
			case fastgltf::PrimitiveType::LineLoop:
				return PrimitiveTopology::LineLoop;
			case fastgltf::PrimitiveType::LineStrip:
				return PrimitiveTopology::LineStrip;
			case fastgltf::PrimitiveType::Triangles:
				return PrimitiveTopology::Triangles;
			case fastgltf::PrimitiveType::TriangleStrip:
				return PrimitiveTopology::TriangleStrip;
			case fastgltf::PrimitiveType::TriangleFan:
				return PrimitiveTopology::TriangleFan;
			default:
				return std::unexpected(Utils::Error("Unsupported glTF primitive topology"));
			}
		}
		
		void ParseExtras(simdjson::dom::object* extras, std::size_t objectIndex, fastgltf::Category category, void* userPointer)
		{
			if (category != fastgltf::Category::Materials)
				return;

			std::vector<MaterialExtras>* materialExtras = static_cast<std::vector<MaterialExtras>*>(userPointer);
			materialExtras->resize(std::max(materialExtras->size(), objectIndex + 1));

			MaterialExtras& target = (*materialExtras)[objectIndex];

			std::string_view surfaceValue;

			if ((*extras)[SurfaceModelKey].get_string().get(surfaceValue) == simdjson::SUCCESS)
			{
				std::optional<Graphics::SurfaceModel> surface = ParseSurfaceModel(surfaceValue);

				if (surface)
					target.surface = surface.value();
				else
					spdlog::warn("Unknown glTF material extras surface value '{}'", surfaceValue);
			}

			bool flipTextures = false;

			if ((*extras)[FlipTexturesKey].get_bool().get(flipTextures) == simdjson::SUCCESS)
				target.flipTextures = flipTextures;
		}
		
		void AssignBytes(std::vector<uint8_t>& destination, std::span<const std::byte> source)
		{
			destination.resize(source.size());
			std::memcpy(destination.data(), source.data(), source.size());
		}

		std::expected<Buffer, Utils::Error> ConvertBuffer(const fastgltf::Buffer& source, const std::filesystem::path& basePath)
		{
			Buffer buffer{};
			buffer.name = std::string(source.name);
			buffer.byteLength = static_cast<uint64_t>(source.byteLength);

			if (const fastgltf::sources::URI* uri = std::get_if<fastgltf::sources::URI>(&source.data))
			{
				if (uri->uri.isDataUri()) // needs custom decoder (maybe later), add if needed
					return std::unexpected(Utils::Error("glTF data URI buffer was left as URI by fastgltf. This importer supports embedded buffers only when fastgltf exposes them as Array, Vector, or ByteView."));

				if (!uri->uri.isLocalPath()) // non-local paths - meaning needs to be downloaded, those are not supported
					return std::unexpected(Utils::Error("Non-local glTF buffer URI is not supported"));

				std::filesystem::path sourceFile = uri->uri.fspath();

				buffer.source = FileBuffer
				{
					.file = sourceFile.empty() ? basePath.filename() : sourceFile,
					.byteOffset = static_cast<uint64_t>(uri->fileByteOffset)
				};

				return buffer;
			}

			if (const fastgltf::sources::Array* array = std::get_if<fastgltf::sources::Array>(&source.data))
			{
				MemoryBuffer memory{};
				AssignBytes(memory.bytes, std::span<const std::byte>(array->bytes.data(), array->bytes.size()));
				buffer.source = std::move(memory);
				return buffer;
			}

			if (const fastgltf::sources::Vector* vector = std::get_if<fastgltf::sources::Vector>(&source.data))
			{
				MemoryBuffer memory{};
				AssignBytes(memory.bytes, std::span<const std::byte>(vector->bytes.data(), vector->bytes.size()));
				buffer.source = std::move(memory);
				return buffer;
			}

			if (const fastgltf::sources::ByteView* byteView = std::get_if<fastgltf::sources::ByteView>(&source.data))
			{
				MemoryBuffer memory{};
				AssignBytes(memory.bytes, std::span<const std::byte>(byteView->bytes.data(), byteView->bytes.size()));
				buffer.source = std::move(memory);
				return buffer;
			}

			if (std::holds_alternative<fastgltf::sources::CustomBuffer>(source.data))
				return std::unexpected(Utils::Error("Custom glTF buffers are not supported"));

			if (std::holds_alternative<fastgltf::sources::Fallback>(source.data))
				return std::unexpected(Utils::Error("Fallback glTF buffers are not supported"));

			return std::unexpected(Utils::Error("Unsupported glTF buffer source"));
		}

		std::expected<BufferAccessor, Utils::Error> ConvertAccessorView(const fastgltf::Asset& asset, size_t accessorIndex)
		{
			if (accessorIndex >= asset.accessors.size())
				return std::unexpected(Utils::Error("glTF accessor index is out of range"));

			const fastgltf::Accessor& accessor = asset.accessors[accessorIndex];

			if (accessor.sparse.has_value())
				return std::unexpected(Utils::Error("Sparse glTF accessors are not representable as a direct buffer view"));

			if (!accessor.bufferViewIndex.has_value())
				return std::unexpected(Utils::Error("glTF accessor has no buffer view"));

			size_t bufferViewIndex = accessor.bufferViewIndex.value();

			if (bufferViewIndex >= asset.bufferViews.size())
				return std::unexpected(Utils::Error("glTF buffer view index is out of range"));

			const fastgltf::BufferView& bufferView = asset.bufferViews[bufferViewIndex];

			if (bufferView.meshoptCompression)
				return std::unexpected(Utils::Error("EXT_meshopt_compression is not representable as a direct buffer view"));

			if (bufferView.bufferIndex >= asset.buffers.size())
				return std::unexpected(Utils::Error("glTF buffer index is out of range"));

			CORE_TRY(elementType, ConvertElementType(accessor.type));
			CORE_TRY(componentType, ConvertComponentType(accessor.componentType));

			uint64_t elementByteLength = static_cast<uint64_t>(fastgltf::getElementByteSize(accessor.type, accessor.componentType));
			uint64_t byteStride = static_cast<uint64_t>(bufferView.byteStride.value_or(elementByteLength));

			BufferView myBufferView = BufferView {
				.byteOffset = static_cast<uint64_t>(bufferView.byteOffset + accessor.byteOffset),
				.byteStride = byteStride,
				.count = static_cast<uint32_t>(accessor.count),
				.elementType = elementType,
				.componentType = componentType,
				.normalized = accessor.normalized
			};

			uint64_t byteLength = myBufferView.GetUsedByteLength();

			if (static_cast<uint64_t>(accessor.byteOffset) + byteLength > static_cast<uint64_t>(bufferView.byteLength))
				return std::unexpected(Utils::Error("glTF accessor exceeds its buffer view"));

			return BufferAccessor
			{
				.buffer = static_cast<uint32_t>(bufferView.bufferIndex),
				.view = myBufferView
			};
		}

		std::expected<std::optional<BufferAccessor>, Utils::Error> ConvertOptionalAttribute(const fastgltf::Asset& asset, const fastgltf::Primitive& primitive, std::string_view name)
		{
			auto attribute = primitive.findAttribute(name);

			if (attribute == primitive.attributes.end())
				return std::optional<BufferAccessor>{};

			CORE_TRY(view, ConvertAccessorView(asset, attribute->accessorIndex));
			return std::optional<BufferAccessor>{ std::move(view) };
		}

		uint32_t FindVertexCount(const fastgltf::Asset& asset, const fastgltf::Primitive& primitive)
		{
			auto position = primitive.findAttribute("POSITION");

			if (position != primitive.attributes.end())
				return static_cast<uint32_t>(asset.accessors[position->accessorIndex].count);

			if (!primitive.attributes.empty())
				return static_cast<uint32_t>(asset.accessors[primitive.attributes.front().accessorIndex].count);

			return 0;
		}

		std::expected<MeshAccessor, Utils::Error> ConvertPrimitive(const fastgltf::Asset& asset, const fastgltf::Primitive& primitive, uint32_t primitiveIndex)
		{
			if (primitive.dracoCompression)
				return std::unexpected(Utils::Error("Draco-compressed glTF primitives are not supported"));

			CORE_TRY_CONTEXT(topology, ConvertTopology(primitive.type), "Failed to convert primitive topology");

			CORE_TRY_CONTEXT(position, ConvertOptionalAttribute(asset, primitive, "POSITION"), "Failed to convert POSITION attribute");
			CORE_TRY_CONTEXT(normal, ConvertOptionalAttribute(asset, primitive, "NORMAL"), "Failed to convert NORMAL attribute");
			CORE_TRY_CONTEXT(tangent, ConvertOptionalAttribute(asset, primitive, "TANGENT"), "Failed to convert TANGENT attribute");
			CORE_TRY_CONTEXT(uv, ConvertOptionalAttribute(asset, primitive, "TEXCOORD_0"), "Failed to convert TEXCOORD_0 attribute");

			uint32_t vertexCount = FindVertexCount(asset, primitive);

			std::optional<BufferAccessor> indices;
			uint32_t indexCount = vertexCount;

			if (primitive.indicesAccessor.has_value())
			{
				CORE_TRY_ASSIGN_CONTEXT(indices, ConvertAccessorView(asset, primitive.indicesAccessor.value()), "Failed to convert indices accessor");
				indexCount = indices.value().view.count;
			}

			return MeshAccessor
			{
				.index = primitiveIndex,
				.vertexCount = vertexCount,
				.indexCount = indexCount,
				.topology = topology,
				.material = primitive.materialIndex.has_value() ? std::make_optional<uint32_t>(static_cast<uint32_t>(primitive.materialIndex.value())) : std::nullopt,
				.indices = std::move(indices),
				.position = std::move(position),
				.normal = std::move(normal),
				.tangent = std::move(tangent),
				.uv = std::move(uv)
			};
		}

		std::optional<std::string> TextureName(const fastgltf::Asset& asset, size_t textureIndex)
		{
			if (textureIndex >= asset.textures.size())
				return std::nullopt;

			const fastgltf::Texture& texture = asset.textures[textureIndex];

			if (!texture.imageIndex.has_value())
				return std::nullopt;

			size_t imageIndex = texture.imageIndex.value();

			if (imageIndex >= asset.images.size())
				return std::nullopt;

			const fastgltf::Image& image = asset.images[imageIndex];

			if (const fastgltf::sources::URI* uri = std::get_if<fastgltf::sources::URI>(&image.data))
			{
				if (!uri->uri.isDataUri() && uri->uri.isLocalPath())
					return uri->uri.fspath().generic_string();
			}

			// Add solution for embedded and buffer view textures
			// if (!image.name.empty())
			// 	return std::string(image.name);

			// if (!texture.name.empty())
			// 	return std::string(texture.name);
			spdlog::warn("glTF texture '{}' is embedded or non-file-backed and is currently ignored", std::string(texture.name));

			return std::nullopt;
		}

		Material ConvertMaterial(const fastgltf::Asset& asset, const fastgltf::Material& source, const MaterialExtras* extras)
		{
			float roughness = static_cast<float>(source.pbrData.roughnessFactor);
			float metallic = static_cast<float>(source.pbrData.metallicFactor);
			float opacity = static_cast<float>(source.pbrData.baseColorFactor[3]);

			float specular[3] = { 1.0f, 1.0f, 1.0f };

			if (source.specular)
			{
				float specularFactor = static_cast<float>(source.specular->specularFactor);

				specular[0] = static_cast<float>(source.specular->specularColorFactor[0]) * specularFactor;
				specular[1] = static_cast<float>(source.specular->specularColorFactor[1]) * specularFactor;
				specular[2] = static_cast<float>(source.specular->specularColorFactor[2]) * specularFactor;
			}

			float transmission = source.transmission ? static_cast<float>(source.transmission->transmissionFactor) : 0.0f;
			
			std::optional<Graphics::SurfaceModel> surface;

			if (extras != nullptr && extras->surface.has_value())
				surface = extras->surface.value();
			else if (source.unlit)
				surface = Graphics::SurfaceModel::Unlit;

			return Material
			{
				.name = std::string(source.name),
				.surface = surface,
				.flipTextures = extras != nullptr ? extras->flipTextures : std::nullopt,
				.color =
				{
					static_cast<float>(source.pbrData.baseColorFactor[0]),
					static_cast<float>(source.pbrData.baseColorFactor[1]),
					static_cast<float>(source.pbrData.baseColorFactor[2])
				},
				.specular = { specular[0], specular[1], specular[2] },
				.shininess = 0.0f,
				.roughness = roughness,
				.metallic = metallic,
				.emission =
				{
					static_cast<float>(source.emissiveFactor[0] * source.emissiveStrength),
					static_cast<float>(source.emissiveFactor[1] * source.emissiveStrength),
					static_cast<float>(source.emissiveFactor[2] * source.emissiveStrength)
				},
				.ior = static_cast<float>(source.ior),
				.transmission = transmission,
				.opacity = opacity,
				.colorTexture = source.pbrData.baseColorTexture.has_value() ? TextureName(asset, source.pbrData.baseColorTexture->textureIndex) : std::nullopt,
				.specularTexture = source.specular && source.specular->specularTexture.has_value() ? TextureName(asset, source.specular->specularTexture->textureIndex) : std::nullopt,
				.shininessTexture = std::nullopt,
				.occlusionTexture = source.occlusionTexture.has_value() ? TextureName(asset, source.occlusionTexture->textureIndex) : std::nullopt,
				.roughnessTexture = std::nullopt,
				.metallicTexture = std::nullopt,
				.rmTexture = source.pbrData.metallicRoughnessTexture.has_value() ? TextureName(asset, source.pbrData.metallicRoughnessTexture->textureIndex) : std::nullopt,
				.ormTexture = source.packedOcclusionRoughnessMetallicTextures 
					? (source.packedOcclusionRoughnessMetallicTextures->occlusionRoughnessMetallicTexture.has_value() 
						? TextureName(asset, source.packedOcclusionRoughnessMetallicTextures->occlusionRoughnessMetallicTexture->textureIndex) 
						: std::nullopt) 
					: std::nullopt,
				.rmoTexture = source.packedOcclusionRoughnessMetallicTextures 
					? (source.packedOcclusionRoughnessMetallicTextures->roughnessMetallicOcclusionTexture.has_value() 
						? TextureName(asset, source.packedOcclusionRoughnessMetallicTextures->roughnessMetallicOcclusionTexture->textureIndex) 
						: std::nullopt) 
					: std::nullopt,
				.emissionTexture = source.emissiveTexture.has_value() ? TextureName(asset, source.emissiveTexture->textureIndex) : std::nullopt,
				.normalTexture = source.normalTexture.has_value() ? TextureName(asset, source.normalTexture->textureIndex) : std::nullopt,
			};
		}

		std::expected<uint32_t, Utils::Error> ConvertMesh(const fastgltf::Asset& asset, size_t sourceMeshIndex, ScenePrefab& prefab, MeshMap& meshMap)
		{
			MeshMap::const_iterator existing = meshMap.find(sourceMeshIndex);

			if (existing != meshMap.end())
				return existing->second;

			if (sourceMeshIndex >= asset.meshes.size())
				return std::unexpected(Utils::Error("glTF mesh index is out of range"));

			const fastgltf::Mesh& source = asset.meshes[sourceMeshIndex];

			Mesh mesh{};
			mesh.primitives.reserve(source.primitives.size());

			for (size_t primitiveSourceIndex = 0; primitiveSourceIndex < source.primitives.size(); primitiveSourceIndex++)
			{
				uint32_t primitiveIndex = static_cast<uint32_t>(prefab.meshPrimitivesAccessors.size());
				CORE_TRY_CONTEXT(primitive, ConvertPrimitive(asset, source.primitives[primitiveSourceIndex], primitiveIndex), "Failed to import glTF mesh primitive {}", primitiveSourceIndex);

				prefab.meshPrimitivesAccessors.push_back(std::move(primitive));
				mesh.primitives.push_back(primitiveIndex);
			}

			uint32_t meshIndex = static_cast<uint32_t>(prefab.meshes.size());
			prefab.meshes.push_back(std::move(mesh));
			meshMap.emplace(sourceMeshIndex, meshIndex);

			return meshIndex;
		}

		std::expected<uint32_t, Utils::Error> ConvertNode(const fastgltf::Asset& asset, size_t sourceNodeIndex, ScenePrefab& prefab, MeshMap& meshMap, NodeMap& nodeMap)
		{
			NodeMap::const_iterator existing = nodeMap.find(sourceNodeIndex);

			if (existing != nodeMap.end())
				return existing->second;

			if (sourceNodeIndex >= asset.nodes.size())
				return std::unexpected(Utils::Error("glTF node index is out of range"));

			const fastgltf::Node& source = asset.nodes[sourceNodeIndex];
			const fastgltf::TRS* trs = std::get_if<fastgltf::TRS>(&source.transform);

			if (trs == nullptr)
				return std::unexpected(Utils::Error("glTF node matrix was not decomposed into TRS"));

			SceneNode node{};
			node.name = std::string(source.name);

			node.translation[0] = trs->translation[0];
			node.translation[1] = trs->translation[1];
			node.translation[2] = trs->translation[2];

			node.rotation[0] = trs->rotation[0];
			node.rotation[1] = trs->rotation[1];
			node.rotation[2] = trs->rotation[2];
			node.rotation[3] = trs->rotation[3];

			node.scale[0] = trs->scale[0];
			node.scale[1] = trs->scale[1];
			node.scale[2] = trs->scale[2];

			if (source.meshIndex.has_value())
			{
				CORE_TRY_CONTEXT(mesh, ConvertMesh(asset, source.meshIndex.value(), prefab, meshMap), "Failed to import glTF mesh for node '{}'", node.name);
				node.mesh = mesh;
			}

			uint32_t nodeIndex = static_cast<uint32_t>(prefab.nodes.size());
			nodeMap.emplace(sourceNodeIndex, nodeIndex);
			prefab.nodes.push_back(std::move(node));

			prefab.nodes[nodeIndex].children.reserve(source.children.size());

			for (size_t childSourceIndex : source.children)
			{
				CORE_TRY_CONTEXT(childIndex, ConvertNode(asset, childSourceIndex, prefab, meshMap, nodeMap), "Failed to import glTF child node");
				prefab.nodes[nodeIndex].children.push_back(childIndex);
			}

			return nodeIndex;
		}
	}

	std::expected<ScenePrefab, Utils::Error> LoadGlTF(const std::filesystem::path& path)
	{
		auto gltfFile = fastgltf::GltfDataBuffer::FromPath(path);

		if (gltfFile.error() != fastgltf::Error::None)
			return std::unexpected(Utils::Error("Failed to open glTF file '{}', error: {}", path.string(), fastgltf::getErrorMessage(gltfFile.error())));

		std::vector<MaterialExtras> materialExtras;

		fastgltf::Parser parser;
		parser.setExtrasParseCallback(ParseExtras);
		parser.setUserPointer(&materialExtras);
		const fastgltf::Options options = fastgltf::Options::DecomposeNodeMatrices;
		auto assetResult = parser.loadGltf(gltfFile.get(), path.parent_path(), options);

		if (assetResult.error() != fastgltf::Error::None)
			return std::unexpected(Utils::Error("Failed to parse glTF file '{}', error: {}", path.string(), fastgltf::getErrorMessage(assetResult.error())));

		fastgltf::Asset asset = std::move(assetResult.get());

		if (asset.scenes.empty())
			return std::unexpected(Utils::Error("glTF file '{}' contains no scenes", path.string()));

		size_t sceneIndex = asset.defaultScene.has_value() ? asset.defaultScene.value() : 0;

		if (sceneIndex >= asset.scenes.size())
			return std::unexpected(Utils::Error("glTF default scene index is out of range"));

		ScenePrefab prefab{};
		prefab.name = path.stem().string();
		prefab.buffers.reserve(asset.buffers.size());

		for (size_t bufferIndex = 0; bufferIndex < asset.buffers.size(); bufferIndex++)
		{
			CORE_TRY_CONTEXT(buffer, ConvertBuffer(asset.buffers[bufferIndex], path), "Failed to import glTF buffer {}", bufferIndex);
			prefab.buffers.push_back(std::move(buffer));
		}

		prefab.materials.reserve(asset.materials.size());

		for (size_t materialIndex = 0; materialIndex < asset.materials.size(); materialIndex++)
		{
			const fastgltf::Material& material = asset.materials[materialIndex];

			const MaterialExtras* extras = materialIndex < materialExtras.size()
				? &materialExtras[materialIndex]
				: nullptr;

			prefab.materials.push_back(ConvertMaterial(asset, material, extras));
		}

		const fastgltf::Scene& scene = asset.scenes[sceneIndex];

		MeshMap meshMap;
		NodeMap nodeMap;

		prefab.roots.reserve(scene.nodeIndices.size());

		for (size_t rootSourceIndex : scene.nodeIndices)
		{
			CORE_TRY_CONTEXT(root, ConvertNode(asset, rootSourceIndex, prefab, meshMap, nodeMap), "Failed to import glTF root node");
			prefab.roots.push_back(root);
		}

		return prefab;
	}
}