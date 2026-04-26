#pragma once
#include <Core/Graphics/Cuda/Bvh/Triangle.hpp>
#include <Core/Graphics/Cuda/Bvh/DeviceBvhNode.hpp>
#include <Core/Graphics/Cuda/Memory/DeviceBuffer1DView.hpp>

namespace Core::Graphics::Cuda
{
	struct DeviceBvhView
	{
		Memory::DeviceBuffer1DView<DeviceBvhNode> nodes;
		Memory::DeviceBuffer1DView<Triangle> triangles;
	};
}