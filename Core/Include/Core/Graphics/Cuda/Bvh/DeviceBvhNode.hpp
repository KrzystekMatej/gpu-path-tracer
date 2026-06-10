#pragma once
#include <cstdint>
#include <Core/Graphics/Cuda/Bvh/BoundingBox.hpp>
#include <Core/Graphics/Cuda/PathTracing/PathTracerDefaults.hpp>

namespace Core::Graphics::Cuda
{
	struct DeviceBvhNode
	{
		DeviceBvhNode(BoundingBox bounds, uint32_t left, uint32_t right, uint32_t first, uint32_t count)
			: bounds(bounds), left(left), right(right), first(first), count(count) {}
		
		__device__ bool IsLeaf() const
		{
			return left == PathTracerDefaults::InvalidIndex && right == PathTracerDefaults::InvalidIndex;
		}

		BoundingBox bounds;
		uint32_t left = PathTracerDefaults::InvalidIndex;
		uint32_t right = PathTracerDefaults::InvalidIndex;
		uint32_t first = 0;
		uint32_t count = 0;
	};
}