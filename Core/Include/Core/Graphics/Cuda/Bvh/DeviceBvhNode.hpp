#pragma once
#include <cstdint>
#include <Core/Graphics/Cuda/Bvh/BoundingBox.hpp>

namespace Core::Graphics::Cuda
{
	struct DeviceBvhNode
	{
		static constexpr uint32_t InvalidIndex = 0xFFFFFFFF;
		BoundingBox bounds;
		uint32_t left = InvalidIndex;
		uint32_t right = InvalidIndex;
		uint32_t first = 0;
		uint32_t count = 0;
	};
}