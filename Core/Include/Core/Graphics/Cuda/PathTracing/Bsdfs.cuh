#pragma once
#include <Core/Graphics/Cuda/Utils/Math.hpp>

namespace Core::Graphics::Cuda::PathTracing
{
    struct BsdfSample
    {
        float3 wi = make_float3(0.0f, 0.0f, 0.0f);
        float3 f = make_float3(0.0f, 0.0f, 0.0f);
        float pdf = 0.0f;
    };

    __forceinline__ __device__ float3 SampleDiffuseWi(float u1, float u2)
    {
        float phi = 2.0f * Math::Pi * u1;
        float cosTheta = sqrtf(u2);
        float sinTheta = sqrtf(1.0f - u2);
        
        return make_float3(sinTheta * cosf(phi), sinTheta * sinf(phi), cosTheta);
    }
    
    __forceinline__ __device__ float3 DiffuseBsdf(float3 baseColor)
    {
        return baseColor * Math::InvPi;
    }

    __forceinline__ __device__ float DiffusePdf(float cosThetaI)
    {
        return cosThetaI * Math::InvPi;
    }

    __forceinline__ __device__ float3 SampleMirrorWi(float3 wo)
    {
        return make_float3(-wo.x, -wo.y, wo.z);
    }

    __forceinline__ __device__ float3 MirrorBsdf(float3 specular)
    {
        return specular;
    }

    __forceinline__ __device__ float MirrorPdf()
    {
        return 1.0f;
    }

    __forceinline__ __device__ float3 SamplePhongWi(float3 wo, float3 specular, float exponent, float u1, float u2)
    {
		float phi = 2.0f * CUDART_PI_F * u1;
		float cosTheta = powf(u2, 1.0f / (exponent + 1.0f));
		float sinTheta = sqrtf(1.0f - cosTheta * cosTheta);

		float3 sampled = make_float3(sinTheta * cosf(phi), sinTheta * sinf(phi), cosTheta);
        float3 reflected = make_float3(-wo.x, -wo.y, wo.z);
        return Math::BuildFrame(reflected).FromLocal(sampled);
    }

    __forceinline__ __device__ float3 PhongBsdf(float3 specular, float normalizationTerm, float phongLobe)
    {
        return specular * normalizationTerm * phongLobe;
    }

    __forceinline__ __device__ float PhongPdf(float exponent, float phongLobe)
    {
        return (exponent + 1.0f) * Math::InvTwoPi * phongLobe;
    }
    
	__device__ __forceinline__ float2 SampleUniformDiskPolar(float2 u)
	{
		float r = sqrtf(u.x);
		float theta = 2.0f * CUDART_PI_F * u.y;
		return make_float2(r * cosf(theta), r * sinf(theta));
	}
	
	__device__ __forceinline__ float3 TrowbridgeSampleWm(float3 w, float2 alpha, float2 u)
	{
		float3 wh = normalize(make_float3(alpha.x * w.x, alpha.y * w.y, w.z));
		if (wh.z < 0)
			wh = -wh;
		
		float3 tangent = (wh.z < 0.999f) ? normalize(cross(wh, make_float3(0.0f, 0.0f, 1.0f))) : make_float3(1.0f, 0.0f, 0.0f);
		float3 bitangent = cross(wh, tangent);

		float2 diskSample = SampleUniformDiskPolar(u);
		float h = sqrtf(1.0f - diskSample.x * diskSample.x);
		diskSample.y = lerp(h, diskSample.y, (1 + wh.z) * 0.5f);
		
		float pz = sqrtf(fmaxf(0.0f, 1.0f - dot(diskSample, diskSample)));
		float3 nh = diskSample.x * tangent + diskSample.y * bitangent + pz * wh;
		return normalize(make_float3(alpha.x * nh.x, alpha.y * nh.y, fmaxf(1e-6f, nh.z)));
	}
}