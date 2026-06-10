#pragma once
#include <Core/Graphics/Cuda/Bvh/Triangle.hpp>
#include <Core/Graphics/Cuda/Bvh/DeviceBvhNode.hpp>
#include <Core/Graphics/Cuda/Runtime/Memory/DeviceBuffer1DView.hpp>

namespace Core::Graphics::Cuda
{
	struct IntersectionBvhView
	{
		Runtime::DeviceBuffer1DView<DeviceBvhNode> nodes;
		Runtime::DeviceBuffer1DView<TriangleIntersection> triangles;
	};
}