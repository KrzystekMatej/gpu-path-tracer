#pragma once
#include <cstdint>
#include <cuda_runtime.h>
#include <curand_kernel.h>

namespace Core::Graphics::Cuda
{
	struct Path
    {
        uint32_t index = 0;
    };

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
		float ior;
		uint32_t depth;
		float3 throughput;
	};

	struct Random
	{
	public:
		__device__ Random(uint64_t seed, uint64_t subsequence, uint64_t offset)
		{
			curand_init(seed, subsequence, offset, &m_State);
		}

		__device__ Random(uint64_t seed, uint64_t subsequence)
		{
			curand_init(seed, subsequence, 0ULL, &m_State);
		}

		__device__ __forceinline__ float NextFloat()
		{
			return curand_uniform(&m_State);
		}
		
		__device__ __forceinline__ float2 NextFloat2()
		{
			float4 values = curand_uniform4(&m_State);
			return make_float2(values.x, values.y);
		}
		
		__device__ __forceinline__ float3 NextFloat3()
		{
			float4 values = curand_uniform4(&m_State);
			return make_float3(values.x, values.y, values.z);
		}

		__device__ __forceinline__ float4 NextFloat4()
		{
			return curand_uniform4(&m_State);
		}
	private:
		curandStatePhilox4_32_10_t m_State;
	};

	struct HitData
	{
		uint32_t triangle;
		float u;
		float v;
	};
}