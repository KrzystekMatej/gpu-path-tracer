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
	private:
		std::unique_ptr<HostBvhNode> BuildRecurse(uint32_t from, uint32_t to, uint32_t depth);

		std::vector<Triangle> m_Triangles;
		std::unique_ptr<HostBvhNode> m_Root;
		uint32_t m_MaxDepth;
		uint32_t m_MinTriangles;
	};
}