#pragma once
#include <cuda_runtime.h>
#include <array>
#include <Core/Graphics/Cuda/PathTracing/Material.hpp>

namespace Core::Graphics::Cuda
{
	struct TriangleIntersection
	{
		float4 edge1;
		float4 edge2;
		float3 v0;
		GlobalShadingModel shadingModel;
	};
	

	struct Vertex
	{
		float4 normal;
		float4 tangent;
		float2 uv;
	};
	
	struct TriangleShading
	{
		Vertex vertices[3];
		float3 geometricNormal;
		uint32_t material;
	};

	struct Triangle
	{
		TriangleIntersection intersection;
		TriangleShading shading;
		float3 positions[3];
	};
}