#pragma once
#include <cuda_runtime.h>
#include <array>
#include <Core/Graphics/Cuda/PathTracing/Material.hpp>

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
		Vertex vertices[3];
		uint32_t materialIndex;
		GlobalShadingModel shadingModel;
	};
}