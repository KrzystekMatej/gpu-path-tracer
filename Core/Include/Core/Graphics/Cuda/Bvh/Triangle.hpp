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

	static_assert(alignof(TriangleIntersection) == 16);
	static_assert(sizeof(TriangleIntersection) == 48);
	
	struct __align__(16) TriangleShading
	{
		float4 normals[3];
		float4 tangents[3];

		float3 geometricNormal;
		uint32_t material;

		float2 uvs[3];
	};

	static_assert(alignof(TriangleShading) == 16);
	static_assert(sizeof(TriangleShading) == 144);

	struct Triangle
	{
		TriangleIntersection intersection;
		TriangleShading shading;
		float3 positions[3];
	};

	struct LightTriangle
	{
		float4 edge1;
		float4 edge2;
		float3 v0;
		uint32_t index;

		float3 geometricNormal;
		float area;

		float2 uv0;
		float2 uvEdge1;
		float2 uvEdge2;

		TextureView<float4> emission;
	};
	
	static_assert(alignof(LightTriangle) == 16);
	static_assert(sizeof(LightTriangle) == 96);
}