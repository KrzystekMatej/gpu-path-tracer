#pragma once
#include <cstdint>
#include <Core/Graphics/Cuda/Bvh/BoundingBox.hpp>

namespace Core::Graphics::Cuda
{
	constexpr uint32_t InvalidNodeIndex = 0xFFFFFFFF;

	struct DeviceBvhNode
	{
		BoundingBox bounds;
		uint32_t left = InvalidNodeIndex;
		uint32_t right = InvalidNodeIndex;
		uint32_t first = 0;
		uint32_t count = 0;
	};
}