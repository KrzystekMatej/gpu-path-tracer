#pragma once
#include <cstdint>
#include <cuda_runtime.h>
#include <curand_kernel.h>

namespace Core::Graphics::Cuda
{
	struct Pixel
	{
		uint32_t index;
	};

	struct Ray
	{
		float3 origin;
		float3 direction;
		float tMin;
		float tMax;
	};

	struct Contribution
	{
		float4 throughput;
	};

	struct Random
	{
		curandStatePhilox4_32_10_t state;
	};

	struct PathFlags
	{
		uint32_t depth;
	};

	struct HitData
	{
		uint32_t path;
		uint32_t triangle;
		uint32_t material;
		float u;
		float v;
	};
}