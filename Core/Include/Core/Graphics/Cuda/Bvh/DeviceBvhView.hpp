#pragma once
#include <Core/Graphics/Cuda/Bvh/Triangle.hpp>
#include <Core/Graphics/Cuda/Bvh/DeviceBvhNode.hpp>
#include <Core/Graphics/Cuda/Runtime/Memory/DeviceBuffer1DView.hpp>

namespace Core::Graphics::Cuda
{
	struct IntersectionBvhView
	{
	public:
		IntersectionBvhView(Runtime::DeviceBuffer1DView<DeviceBvhNode> nodes, Runtime::DeviceBuffer1DView<TriangleIntersection> triangles)
			: m_Nodes(nodes), m_Triangles(triangles) {}
		
		__device__ const DeviceBvhNode& GetNode(uint32_t index) const
		{
			return m_Nodes.At(index);
		}

		__device__ const TriangleIntersection& GetTriangle(uint32_t index) const
		{
			return m_Triangles.At(index);
		}
	private:
		Runtime::DeviceBuffer1DView<DeviceBvhNode> m_Nodes;
		Runtime::DeviceBuffer1DView<TriangleIntersection> m_Triangles;
	};
}