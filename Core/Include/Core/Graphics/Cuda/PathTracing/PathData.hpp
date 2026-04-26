#pragma once
#include <cstdint>
#include <cuda_runtime.h>
#include <curand_kernel.h>

namespace Core::Graphics::Cuda
{
	struct Sample
	{
		uint32_t pixel;
		uint32_t sample;
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
		uint32_t pathId;
		uint32_t triangleID;
		float u;
		float v;
		uint32_t materialID;
		float3 Ng;
	};
}