#pragma once
#include <cuda_runtime.h>

namespace Core::Graphics::Cuda
{
	struct BoundingBox
	{
		float3 min;
		float3 max;
	};
}