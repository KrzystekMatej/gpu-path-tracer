#pragma once
#include <cuda_runtime.h>
#include <Core/External/Glm.hpp>

namespace Core::Graphics::Cuda::Utils::Glm
{
	inline float2 ToFloat2(const glm::vec2& vec)
	{
		return make_float2(vec.x, vec.y);
	}
	
	inline float3 ToFloat3(const glm::vec3& vec)
	{
		return make_float3(vec.x, vec.y, vec.z);
	}

	inline float4 ToFloat4(const glm::vec4& vec)
	{
		return make_float4(vec.x, vec.y, vec.z, vec.w);
	}
}