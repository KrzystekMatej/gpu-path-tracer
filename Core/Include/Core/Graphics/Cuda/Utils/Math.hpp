#pragma once
#include <cuda_runtime.h>

namespace Core::Graphics::Cuda
{
	inline __forceinline__ __host__ __device__ float At(float2 number, int index)
	{
		switch (index)
		{
			case 0:
				return number.x;
			case 1:
				return number.y;
			default:
				return 0.0f;
		}
	}

	inline __forceinline__ __host__ __device__ float At(float3 number, int index)
	{
		switch (index)
		{
			case 0:
				return number.x;
			case 1:
				return number.y;
			case 2:
				return number.z;
			default:
				return 0.0f;
		}
	}

	inline __forceinline__ __host__ __device__ float At(float4 number, int index)
	{
		switch (index)
		{
			case 0:
				return number.x;
			case 1:
				return number.y;
			case 2:
				return number.z;
			case 3:
				return number.w;
			default:
				return 0.0f;
		}
	}

	inline __forceinline__ __host__ __device__ float3 operator+(const float3& a, const float3& b)
	{
		return make_float3(a.x + b.x, a.y + b.y, a.z + b.z);
	}

	inline __forceinline__ __host__ __device__ float4 operator+(const float4& a, const float4& b)
	{
		return make_float4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
	}

	inline __forceinline__ __host__ __device__ float3 operator-(const float3& a, const float3& b)
	{
		return make_float3(a.x - b.x, a.y - b.y, a.z - b.z);
	}

	inline __forceinline__ __host__ __device__ float4 operator-(const float4& a, const float4& b)
	{
		return make_float4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
	}

	inline __forceinline__ __host__ __device__ float3 operator*(const float3& a, const float3& b)
	{
		return make_float3(a.x * b.x, a.y * b.y, a.z * b.z);
	}

	inline __forceinline__ __host__ __device__ float4 operator*(const float4& a, const float4& b)
	{
		return make_float4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
	}

	inline __forceinline__ __host__ __device__ float3 operator/(const float3& a, const float3& b)
	{
		return make_float3(a.x / b.x, a.y / b.y, a.z / b.z);
	}

	inline __forceinline__ __host__ __device__ float4 operator/(const float4& a, const float4& b)
	{
		return make_float4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
	}

	inline __forceinline__ __host__ __device__ float3 operator*(const float3& a, float b)
	{
		return make_float3(a.x * b, a.y * b, a.z * b);
	}

	inline __forceinline__ __host__ __device__ float4 operator*(const float4& a, float b)
	{
		return make_float4(a.x * b, a.y * b, a.z * b, a.w * b);
	}

	inline __forceinline__ __host__ __device__ float3 operator/(const float3& a, float b)
	{
		return make_float3(a.x / b, a.y / b, a.z / b);
	}

	inline __forceinline__ __host__ __device__ float4 operator/(const float4& a, float b)
	{
		return make_float4(a.x / b, a.y / b, a.z / b, a.w / b);
	}
}