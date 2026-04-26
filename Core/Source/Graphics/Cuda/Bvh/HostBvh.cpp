#include <Core/Graphics/Cuda/Bvh/HostBvh.hpp>
#include <algorithm>
#include <Core/Graphics/Cuda/Utils/Math.hpp>
#include <cmath>

namespace Core::Graphics::Cuda
{
	namespace
	{
		float3 min(const float3& a, const float3& b)
		{
			return make_float3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
		}

		float3 max(const float3& a, const float3& b)
		{
			return make_float3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
		}

		uint32_t EstimateMaxTrianglesPerLeaf(size_t triangleCount)
		{
			if (triangleCount < 5'000)   return 4;
			if (triangleCount < 50'000)   return 6;
			if (triangleCount < 500'000)   return 8;
			if (triangleCount < 2'000'000)   return 12;
			if (triangleCount < 10'000'000)   return 16;
			return 24;
		}

		uint32_t EstimateMaxDepth(size_t triangleCount)
		{
			return static_cast<uint32_t>(std::max(1.0, std::ceil(2 * std::log2(triangleCount))));
		}
	}

	HostBvh::HostBvh(std::vector<Triangle> triangles)
		: m_Triangles(std::move(triangles)), 
		m_MaxDepth(EstimateMaxDepth(m_Triangles.size())), 
		m_MinTriangles(EstimateMaxTrianglesPerLeaf(m_Triangles.size()))
	{
		m_Root = BuildRecurse(0, static_cast<uint32_t>(m_Triangles.size()), 0);
	}

	std::unique_ptr<HostBvhNode> HostBvh::BuildRecurse(uint32_t first, uint32_t count, uint32_t depth)
	{
		auto node = std::make_unique<HostBvhNode>();
		node->first = first;
		node->count = count;
		BoundingBox bounds{};
		for (uint32_t i = first; i < first + count; i++)
		{
			const Triangle& triangle = m_Triangles[i];
			for (const Vertex& vertex : triangle.vertices)
			{
				bounds.min = min(bounds.min, vertex.position);
				bounds.max = max(bounds.max, vertex.position);
			}
		}

		node->bounds = bounds;
		if (depth >= m_MaxDepth || count <= m_MinTriangles)
			return node;

		uint32_t axis = 0;
		float extentX = bounds.max.x - bounds.min.x;
		float extentY = bounds.max.y - bounds.min.y;
		float extentZ = bounds.max.z - bounds.min.z;
		if (extentY > extentX && extentY > extentZ)
			axis = 1;
		else if (extentZ > extentX && extentZ > extentY)
			axis = 2;
		uint32_t mid = first + (count / 2);
		std::nth_element(m_Triangles.begin() + first, m_Triangles.begin() + mid, m_Triangles.begin() + first + count,
			[axis](const Triangle& a, const Triangle& b)
			{
				float3 centerA = (a.vertices[0].position + a.vertices[1].position + a.vertices[2].position) / 3.0f;
				float3 centerB = (b.vertices[0].position + b.vertices[1].position + b.vertices[2].position) / 3.0f;
				return At(centerA, axis) < At(centerB, axis);
			});
		node->left = BuildRecurse(first, mid - first, depth + 1);
		node->right = BuildRecurse(mid, count - (mid - first), depth + 1);
		return node;
	}
}