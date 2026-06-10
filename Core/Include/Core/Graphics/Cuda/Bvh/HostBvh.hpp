#pragma once
#include <memory>
#include <vector>
#include <Core/Graphics/Cuda/Bvh/Triangle.hpp>
#include <Core/Graphics/Cuda/Bvh/BoundingBox.hpp>

namespace Core::Graphics::Cuda
{
	struct HostBvhNode
	{
		std::unique_ptr<HostBvhNode> left = nullptr;
		std::unique_ptr<HostBvhNode> right = nullptr;
		uint32_t first = 0;
		uint32_t count = 0;
		BoundingBox bounds;
	};
	
	class HostBvh
	{
	public:
		HostBvh(std::vector<Triangle> triangles);
		const HostBvhNode& GetRoot() const { return *m_Root; }
		const std::vector<Triangle>& GetTriangles() const { return m_Triangles; }
		uint32_t GetMaxDepth() const { return m_MaxDepth; }
		uint32_t GetMinTriangles() const { return m_MinTriangles; }
		uint32_t GetDepth() const { return m_Depth; }
		uint32_t GetNodeCount() const { return m_NodeCount; }
	private:
		std::unique_ptr<HostBvhNode> BuildRecurse(uint32_t from, uint32_t to, uint32_t depth);

		std::vector<Triangle> m_Triangles;
		std::unique_ptr<HostBvhNode> m_Root;
		uint32_t m_MaxDepth = 0;
		uint32_t m_MinTriangles = 0;
		uint32_t m_Depth = 0;
		uint32_t m_NodeCount = 0;
	};
}