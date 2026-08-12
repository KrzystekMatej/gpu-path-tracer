#pragma once
#include <optional>
#include <cstddef>
#include <Core/Import/Buffers/Buffer.hpp>
#include <Core/Graphics/Common/Vertex.hpp>
#include <Core/Graphics/Common/Material.hpp>

namespace Core::Import
{
	struct BufferAccessor
	{
		uint32_t buffer = 0;
		BufferView view;
	};

	struct MeshAccessor
	{
		uint32_t index = 0;

		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;

		PrimitiveTopology topology = PrimitiveTopology::Triangles;

		std::optional<uint32_t> material;

		std::optional<BufferAccessor> indices;
		std::optional<BufferAccessor> position;
		std::optional<BufferAccessor> normal;
		std::optional<BufferAccessor> tangent;
		std::optional<BufferAccessor> uv;
	};
	
	struct MeshPrimitive
	{
		std::vector<Graphics::Vertex> vertices;
		std::vector<uint32_t> indices;
	};

	inline BufferView IndexView(uint32_t count)
	{
		return BufferView
		{
			.byteOffset = 0,
			.byteStride = sizeof(uint32_t),
			.count = count,
			.elementType = ElementType::Scalar,
			.componentType = ComponentType::UnsignedInt,
			.normalized = false
		};
	}
	
	inline BufferView VertexView(size_t byteOffset, uint32_t count, ElementType elementType)
	{
		return BufferView
		{
			.byteOffset = byteOffset,
			.byteStride = sizeof(Graphics::Vertex),
			.count = count,
			.elementType = elementType,
			.componentType = ComponentType::Float,
			.normalized = false
		};
	}

	inline BufferView PositionView(uint32_t count) { return VertexView(offsetof(Graphics::Vertex, position), count, ElementType::Vec3); }
	inline BufferView NormalView(uint32_t count) { return VertexView(offsetof(Graphics::Vertex, normal), count, ElementType::Vec3); }
	inline BufferView TangentView(uint32_t count) { return VertexView(offsetof(Graphics::Vertex, tangent), count, ElementType::Vec4); }
	inline BufferView UVView(uint32_t count) { return VertexView(offsetof(Graphics::Vertex, uv), count, ElementType::Vec2); }
	
	struct Material
	{
		// missing factors (strengths) are assumed to be 1.0f
		std::string name;

		std::optional<Graphics::SurfaceModel> surface;
		std::optional<bool> flipTextures;

		std::array<float, 3> color;
		std::array<float, 3> specular;
		float shininess;
		float roughness;
		float metallic;
		std::array<float, 3> emission;
		float ior;
		float transmission;
		float opacity;

		std::optional<std::string> colorTexture;
		std::optional<std::string> specularTexture;
		std::optional<std::string> shininessTexture;
		std::optional<std::string> occlusionTexture;
		std::optional<std::string> roughnessTexture;
		std::optional<std::string> metallicTexture;
		std::optional<std::string> rmTexture;
		std::optional<std::string> ormTexture;
		std::optional<std::string> rmoTexture;
		std::optional<std::string> emissionTexture;
		std::optional<std::string> normalTexture;
	};

	struct Mesh
	{
		std::vector<uint32_t> primitives;
	};

	struct SceneNode
	{
		std::string name;

		std::array<float, 3> translation;
		std::array<float, 4> rotation;
		std::array<float, 3> scale;

		std::optional<uint32_t> mesh;
		std::vector<uint32_t> children;
	};

	struct ScenePrefab
	{
		std::string name;

		std::vector<Buffer> buffers;
		std::vector<SceneNode> nodes;
		std::vector<Mesh> meshes;
		std::vector<MeshAccessor> meshPrimitivesAccessors;
		std::vector<Material> materials;
		std::vector<uint32_t> roots;
	};
}