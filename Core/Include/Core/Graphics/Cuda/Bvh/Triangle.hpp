#pragma once
#include <cuda_runtime.h>
#include <array>

namespace Core::Graphics::Cuda
{
	struct Vertex
	{
		float3 position;
		float3 normal;
		float4 tangent;
		float2 uv;
	};

	struct Triangle
	{
		std::array<Vertex, 3> vertices;
		uint32_t materialIndex;
	};
}