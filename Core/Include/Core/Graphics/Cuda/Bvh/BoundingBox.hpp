#pragma once

#include <limits>
#include <helper_math.h>
#include <cuda_runtime.h>

namespace Core::Graphics::Cuda
{
	struct BoundingBox
	{
		float3 min = make_float3(-std::numeric_limits<float>::max());
		float3 max = make_float3(std::numeric_limits<float>::max());

		static BoundingBox Empty() 
		{
			BoundingBox bounds;
			bounds.min = make_float3(std::numeric_limits<float>::max());
			bounds.max = make_float3(-std::numeric_limits<float>::max());
			return bounds;
		}
		
		static BoundingBox Infinite()
		{
			BoundingBox bounds;
			bounds.min = make_float3(-std::numeric_limits<float>::max());
			bounds.max = make_float3(std::numeric_limits<float>::max());
			return bounds;
		}
	};
}