#include <Core/Graphics/Cuda/PathTracing/Kernels.hpp>
#include <helper_math.h>
#include <cuda_runtime.h>
#include <cstdio>
#include <Core/Graphics/Cuda/Memory/ViewUtils.cuh>
#include <Core/Graphics/Cuda/Memory/Stack.hpp>
#include <Core/Graphics/Cuda/Bvh/BvhDefaults.hpp>
#include <Core/Graphics/Cuda/PathTracing/PathTracerDefaults.hpp>
#include <math_constants.h>
#include <Core/Graphics/Cuda/Utils/Math.hpp>
#include <Core/Graphics/Common/Material.hpp>

namespace Core::Graphics::Cuda::Kernels
{
	#define TPB_DIM_1D 16 * 16
	#define TPB_DIM_2D 16
	#define SEED 1234

	uint32_t GetBlockCount(uint32_t dataSize, uint32_t blockSize)
	{
		return (dataSize + blockSize - 1) / blockSize;
	}

	__global__ void ClearKernel(uchar4 color, Memory::DeviceBuffer1DView<uchar4> framebuffer)
	{
		uint32_t pixelIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (pixelIndex >= framebuffer.size) return;

		Memory::At(framebuffer, pixelIndex) = color;
	}

	__global__ void InitializePathsKernel(
		DeviceCamera camera, 
		uint32_t width, 
		uint32_t height, 
		uint32_t sampleGridSize, 
		uint32_t spp, 
		uint32_t generateCount,
		PathPoolView pathPool, 
		Memory::DeviceQueueView<uint32_t> rayQueue)
	{
		const uint32_t sampleIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (sampleIndex >= generateCount) return;

		const uint32_t pixelIndex = sampleIndex / spp;
		const uint32_t sampleGridIndex = sampleIndex % spp;

		const uint32_t x = pixelIndex % width;
		const uint32_t y = pixelIndex / width;

		curandStatePhilox4_32_10_t state;

		curand_init(SEED, sampleIndex, 0ULL, &state);
		
		const uint32_t sampleGridX = sampleGridIndex % sampleGridSize;
		const uint32_t sampleGridY = sampleGridIndex / sampleGridSize;

		const float jitterX = curand_uniform(&state);
		const float jitterY = curand_uniform(&state);

		const float u = (x + (sampleGridX + jitterX) / sampleGridSize) / width;
		const float v = (y + (sampleGridY + jitterY) / sampleGridSize) / height;

		float3 rayDirection = normalize(camera.lowerLeftCorner + u * camera.horizontal + v * camera.vertical - camera.origin);
		
		// sampleIndex == pathIndex in this kernel
		Pixel& pixel = Memory::At(pathPool.pixels, sampleIndex);
		pixel.index = pixelIndex;

		Ray& ray = Memory::At(pathPool.rays, sampleIndex);
		ray.direction = rayDirection;
		ray.origin = camera.origin;
		ray.tMin = PathTracerDefaults::MinT;
		ray.tMax = PathTracerDefaults::MaxT;
		ray.ior = 1.0f;
		
		Contribution& contribution = Memory::At(pathPool.contributions, sampleIndex);
		contribution.throughput = make_float4(1.0f);
		
		Random& random = Memory::At(pathPool.randoms, sampleIndex);
		random.state = state;
		
		PathFlags& pathFlags = Memory::At(pathPool.pathFlags, sampleIndex);
		pathFlags.depth = 0;

		Memory::At(rayQueue, sampleIndex) = sampleIndex;
	}
	
	__device__ __forceinline__ bool AabbIntersect(const BoundingBox& bounds, const float3& origin, const float3& invDirection, float tMin, float tMax)
	{
		float3 t0 = (bounds.min - origin) * invDirection;
		float3 t1 = (bounds.max - origin) * invDirection;

		float3 lo = fminf(t0, t1);
		float3 hi = fmaxf(t0, t1);

		tMin = fmaxf(tMin, fmaxf(lo.x, fmaxf(lo.y, lo.z)));
		tMax = fminf(tMax, fminf(hi.x, fminf(hi.y, hi.z)));
		return tMax >= tMin;
	}

	__device__ __forceinline__ bool IntersectTriangle(
		const Triangle& triangle,
		const float3& origin,
		const float3& direction,
		float tMin,
		float tMax,
		float& t,
		float& u,
		float& v)
	{
		const float3 v0 = triangle.vertices[0].position;
		const float3 v1 = triangle.vertices[1].position;
		const float3 v2 = triangle.vertices[2].position;

		const float3 e1 = v1 - v0;
		const float3 e2 = v2 - v0;
		const float3 p = cross(direction, e2);

		const float det = dot(e1, p);
		const float epsilon = 1e-7f;

		if (fabs(det) < epsilon)
			return false;

		const float invDet = 1.0f / det;

		const float3 s = origin - v0;
		u = dot(s, p) * invDet;

		if (u < 0.0f || u > 1.0f)
			return false;

		const float3 q = cross(s, e1);
		v = dot(direction, q) * invDet;

		if (v < 0.0f || u + v > 1.0f)
			return false;

		t = dot(e2, q) * invDet;

		return t >= tMin && t <= tMax;
	}
	
	__global__ void IntersectRaysWithSceneKernel(
		PathPoolView pathPool, 
		Memory::DeviceQueueView<uint32_t> rayQueue, 
		DeviceBvhView bvh, 
		Memory::DeviceQueueView<HitData> hitQueue, 
		Memory::DeviceQueueView<uint32_t> regenQueue)
	{
		const uint32_t queueIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (queueIndex >= *rayQueue.counter.value) return;
		
		uint32_t pathIndex = Memory::At(rayQueue, queueIndex);
		const Ray ray = Memory::At(pathPool.rays, pathIndex);
		const float3 invDirection = make_float3(1.0f) / ray.direction;
		//TODO: use templates to accomodate stack sizes for different (shallower) BVH depths
		Memory::Stack<uint32_t, BvhDefaults::MaxDepth + 2> stack;
		stack.Push(0);
		HitData closestHit;
		closestHit.triangle = PathTracerDefaults::InvalidIndex;
		float closestT = ray.tMax;

		while (!stack.IsEmpty())
		{
			uint32_t nodeIndex = stack.Pop();
			const DeviceBvhNode node = Memory::At(bvh.nodes, nodeIndex);

			if (!AabbIntersect(node.bounds, ray.origin, invDirection, ray.tMin, closestT))
				continue;

			if (node.left == PathTracerDefaults::InvalidIndex && node.right == PathTracerDefaults::InvalidIndex)
			{
				for (uint32_t i = 0; i < node.count; i++)
				{
					const Triangle triangle = Memory::At(bvh.triangles, node.first + i);
					float t, u, v;
					if (IntersectTriangle(triangle, ray.origin, ray.direction, ray.tMin, closestT, t, u, v) && t < closestT)
					{
						closestT = t;
						
						closestHit.triangle = node.first + i;
						closestHit.material = triangle.materialIndex;
						closestHit.u = u;
						closestHit.v = v;
					}
				}
			}
			else
			{
				stack.Push(node.left);
				stack.Push(node.right);
			}
		}
		
		if (closestHit.triangle != PathTracerDefaults::InvalidIndex)
		{
			closestHit.path = pathIndex;
			Memory::Push(hitQueue, closestHit);
			Memory::At(pathPool.rays, pathIndex).tMax = closestT;
		}
		else
		{
			Memory::Push(regenQueue, pathIndex);
		}
	}

	__device__ __forceinline__ float3 GetGeometricNormal(const Triangle& triangle)
	{
		float3 v0 = triangle.vertices[0].position;
		float3 v1 = triangle.vertices[1].position;
		float3 v2 = triangle.vertices[2].position;

		float3 e1 = v1 - v0;
		float3 e2 = v2 - v0;

		return normalize(cross(e1, e2));
	}

	inline __forceinline__ __host__ __device__ Math::Frame GetShadingFrame(const Triangle& triangle, float4 normalMapSample, float b0, float b1, float b2)
	{
		float3 baseNormal = normalize(
			b0 * triangle.vertices[0].normal +
			b1 * triangle.vertices[1].normal +
			b2 * triangle.vertices[2].normal
		);

		float4 interpolatedTangent =
			b0 * triangle.vertices[0].tangent +
			b1 * triangle.vertices[1].tangent +
			b2 * triangle.vertices[2].tangent;

		float3 baseTangent = make_float3(
			interpolatedTangent.x,
			interpolatedTangent.y,
			interpolatedTangent.z
		);

		float tangentSign = interpolatedTangent.w < 0.0f ? -1.0f : 1.0f;
		Math::Frame baseFrame = Math::BuildFrame(baseNormal, baseTangent, tangentSign);

		float3 tangentSpaceNormal = normalize(make_float3(
			normalMapSample.x * 2.0f - 1.0f,
			normalMapSample.y * 2.0f - 1.0f,
			normalMapSample.z * 2.0f - 1.0f
		));

		float3 shadingNormal = normalize(
			tangentSpaceNormal.x * baseFrame.tangent +
			tangentSpaceNormal.y * baseFrame.bitangent +
			tangentSpaceNormal.z * baseFrame.normal
		);

		return Math::BuildFrame(shadingNormal, baseTangent, tangentSign);
	}

	__device__ __forceinline__ float3 SampleCosineWeightedHemisphere(float2 u)
	{
		float phi = 2.0f * CUDART_PI_F * u.x;
		float cosTheta = sqrtf(u.y);
		float sinTheta = sqrtf(1.0f - u.y);
		
		return make_float3(sinTheta * cosf(phi), sinTheta * sinf(phi), cosTheta);
	}

	__device__ __forceinline__ float3 SamplePhongLobe(float3 reflected, float exponent, float2 u)
	{
		float phi = 2.0f * CUDART_PI_F * u.x;
		float cosTheta = powf(u.y, 1.0f / (exponent + 1.0f));
		float sinTheta = sqrtf(1.0f - cosTheta * cosTheta);

		float3 sampled = make_float3(sinTheta * cosf(phi), sinTheta * sinf(phi), cosTheta);
		
		return Math::BuildFrame(reflected).FromLocal(sampled);
	}

	__device__ __forceinline__ float FresnelDielectric(float cosThetaI, float eta)
	{
		cosThetaI = clamp(cosThetaI, -1.f, 1.f);
		if (cosThetaI < 0) {
			eta = 1 / eta;
			cosThetaI = -cosThetaI;
		}

		float sin2ThetaI = 1 - Math::Sqr(cosThetaI);
		float sin2ThetaT = sin2ThetaI / Math::Sqr(eta);
		if (sin2ThetaT >= 1)
			return 1.f;
		float cosThetaT = Math::SafeSqrt(1 - sin2ThetaT);

		float parallelRatio = (eta * cosThetaI - cosThetaT) / (eta * cosThetaI + cosThetaT);
		float perpendicularRatio = (cosThetaI - eta * cosThetaT) / (cosThetaI + eta * cosThetaT);
		return (Math::Sqr(parallelRatio) + Math::Sqr(perpendicularRatio)) * 0.5f;
	}

	__device__ __forceinline__ float FresnelComplex(float cosThetaI, Math::Complex eta)
	{
		cosThetaI = clamp(cosThetaI, 0.0f, 1.0f);
		float sin2ThetaI = 1.0f - cosThetaI * cosThetaI;
		Math::Complex sinThetaT = sin2ThetaI / (eta * eta);
		Math::Complex cosThetaT = Math::Sqrt(1.0f - sinThetaT * sinThetaT);
		Math::Complex parallelRatio = (eta * cosThetaI - cosThetaT) / (eta * cosThetaI + cosThetaT);
		Math::Complex perpendicularRatio = (cosThetaI - eta * cosThetaT) / (cosThetaI + eta * cosThetaT);
		return (Math::Normalize(parallelRatio) + Math::Normalize(perpendicularRatio)) * 0.5f;
	}

	__device__ __forceinline__ float2 SampleUniformDiskPolar(float2 u)
	{
		float r = sqrtf(u.x);
		float theta = 2.0f * CUDART_PI_F * u.y;
		return make_float2(r * cosf(theta), r * sinf(theta));
	}
	
	__device__ __forceinline__ float3 TrowbridgeSampleWm(float3 w, float2 alpha, float2 u)
	{
		float3 wh = normalize(float3(alpha.x * w.x, alpha.y * w.y, w.z));
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

	__device__ __forceinline__ float Lambda(float3 w, float alpha)
	{
		float tan2Theta = Math::Tan2Theta(w);
		if (tan2Theta == CUDART_INF_F) 
			return 0.0f;
		float alpha2 = Math::Sqr(Math::CosPhi(w) * alpha) + Math::Sqr(Math::SinPhi(w) * alpha);
		return (sqrtf(1.0f + alpha2 * tan2Theta) - 1.0f) * 0.5f;
	}
	
	__device__ __forceinline__ float G1(float3 w, float alpha)
	{
		return 1.0f / (1.0f + Lambda(w, alpha));
	}
	
	__device__ __forceinline__ float G(float3 wo, float3 wi, float alpha)
	{
		return 1 / (1 + Lambda(wo, alpha) + Lambda(wi, alpha));
	}
	
	__device__ __forceinline__ float D(float3 wm, float alpha)
	{
		float tan2Theta = Math::Tan2Theta(wm);
		if (tan2Theta == CUDART_INF_F)
			return 0.0f;
		float cos4Theta = Math::Sqr(Math::Cos2Theta(wm));
		if (cos4Theta < 1e-16f) return 0.0f;
		float e = tan2Theta * (Math::Sqr(Math::CosPhi(wm) / alpha) + Math::Sqr(Math::SinPhi(wm) / alpha));
		return 1 / (CUDART_PI_F * Math::Sqr(alpha) * cos4Theta * Math::Sqr(1 + e));
	}

	__device__ __forceinline__ float D(float3 w, float3 wm, float alpha)
	{
		return G1(w, alpha) / Math::AbsCosTheta(w) * D(wm, alpha) * fabsf(dot(w, wm));
	}

	__device__ __forceinline__ float3 FresnelSchlick(float3 f0, float cosThetaH)
	{
		return f0 + (1.0f - f0) * powf(1.0f - cosThetaH, 5.0f);
	}

	__device__ __forceinline__ void SampleConductorBsdf(float3 wo, float3 normal, float alpha, float eta, float k, curandStatePhilox4_32_10_t& state)
	{
		if (alpha < 1e-3f)
		{
			float3 wi(-wo.x, -wo.y, wo.z);
			float cosThetaI = Math::AbsCosTheta(wi);
			float brdf = FresnelComplex(cosThetaI, Math::Complex(eta, k)) / cosThetaI;
			float pdf = 1.0f;
			return;
		}

		if (wo.z == 0.0f) return;
		float3 wm = TrowbridgeSampleWm(wo, make_float2(alpha, alpha), make_float2(curand_uniform(&state), curand_uniform(&state)));
		float3 wi = reflect(-wo, wm);
		
		if (!Math::SameHemisphere(wi, normal)) return;
		
		float pdf = D(wo, wm, alpha) / (4.0f * fabsf(dot(wo, wm)));
		float cosThetaO = Math::AbsCosTheta(wo);
		float cosThetaI = Math::AbsCosTheta(wi);
		if (cosThetaI == 0.0f || cosThetaO == 0.0f) return;
		float F = FresnelComplex(fabsf(dot(wi, wm)), Math::Complex(eta, k));
		float f = F * D(wm, alpha) * G(wo, wi, alpha) / (4.0f * cosThetaI * cosThetaO);
	}
	
	__device__ __forceinline__ bool Refract(float3 wi, float3 normal, float eta, float& etap, float3& wt)
	{
		float cosThetaI = dot(normal, wi);
		if (cosThetaI < 0) {
			eta = 1 / eta;
			cosThetaI = -cosThetaI;
			normal = -normal;
		}

		float sin2ThetaI = fmaxf(0.0f, 1 - Math::Sqr(cosThetaI));
		float sin2ThetaT = sin2ThetaI / Math::Sqr(eta);
		if (sin2ThetaT >= 1)
			return false;
		float cosThetaT = Math::SafeSqrt(1 - sin2ThetaT);
		
		wt = -wi / eta + (cosThetaI / eta - cosThetaT) * normal;
		etap = eta;
		return true;
	}
	
	__device__ __forceinline__ void SampleDielectricBsdf(float3 wo, float3 normal, float alpha, float eta, float k, curandStatePhilox4_32_10_t& state)
	{
		if (eta == 1 || alpha < 1e-3f)
		{
			float r = FresnelDielectric(Math::CosTheta(wo), eta);
			float t = 1 - r;
			
			float pr = r / (r + t);
			if (curand_uniform(&state) < pr)
			{
				float3 wi(-wo.x, -wo.y, wo.z);
				float pdf = pr;
				float brdf = r / Math::AbsCosTheta(wi);
				return;
			}
			float3 wi;
			float etap;
			if (!Refract(wo, float3(0, 0, 1), eta, etap, wi)) return;
			float brdf = t / Math::AbsCosTheta(wi);
			float pdf = 1.0f - pr;
			return;
		}
		
		
		float3 wm = TrowbridgeSampleWm(wo, make_float2(alpha, alpha), make_float2(curand_uniform(&state), curand_uniform(&state)));
		float r = FresnelDielectric(dot(wo, wm), eta);
		float t = 1 - r;

		float pr = r / (r + t);
		if (curand_uniform(&state) < pr)
		{
			float3 wi = reflect(-wo, wm);
			if (!Math::SameHemisphere(wo, wi)) return;
			float pdf = D(wo, wm, alpha) / (4.0f * fabsf(dot(wo, wm))) * pr;
			float brdf = r * D(wm, alpha) * G(wo, wi, alpha) / (4.0f * Math::AbsCosTheta(wi) * Math::AbsCosTheta(wo));
			return;
		}
		
		float3 wi;
		float etap;
		if (!Refract(wo, wm, eta, etap, wi)) return;
		if (Math::SameHemisphere(wo, wi) || wi.z == 0.0f) return;

		float denom = Math::Sqr(dot(wi, wm) + dot(wo, wm) / etap);
		float dwm_dwi = fabsf(dot(wi, wm)) / denom;
		float pdf = D(wo, wm, alpha) * dwm_dwi * (1 - pr);
		float brdf = t * D(wm, alpha) * G(wo, wi, alpha) * fabsf(dot(wi, wm) * dot(wo, wm) / (Math::AbsCosTheta(wi) * Math::AbsCosTheta(wo) * denom));
		return;
	}

	__device__ __forceinline__ float luminance(float3 color)
	{
		return 0.2126f * color.x + 0.7152f * color.y + 0.0722f * color.z;
	}

	__global__ void ResolveHitsKernel(
		PathPoolView pathPool, 
		Memory::DeviceQueueView<HitData> hitQueue, 
		Memory::DeviceBuffer1DView<Triangle> triangles,
		Memory::DeviceBuffer1DView<Material> materials,
		uint32_t pathDepthLimit,
		Memory::DeviceQueueView<uint32_t> regenQueue,
		Memory::DeviceBuffer1DView<float4> accumulationBuffer,
		Memory::DeviceQueueView<uint32_t> nextRayQueue)
	{
		const uint32_t queueIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (queueIndex >= *hitQueue.counter.value) return;
		
		const HitData hitData = Memory::At(hitQueue, queueIndex);
		const Material material = Memory::At(materials, hitData.material);
		Contribution& contribution = Memory::At(pathPool.contributions, hitData.path);
		bool pathTerminated = false;

		switch (material.shadingModel)
		{
			//TODO: unify throughput access?
			case GlobalShadingModel::Normal:
			{
				Triangle triangle = Memory::At(triangles, hitData.triangle);
				float b0 = 1.0f - hitData.u - hitData.v;
				float b1 = hitData.u;
				float b2 = hitData.v;
				float2 uv =
					b0 * triangle.vertices[0].uv +
					b1 * triangle.vertices[1].uv +
					b2 * triangle.vertices[2].uv;
				
				Math::Frame shadingFrame = GetShadingFrame(triangle, Memory::Sample(material.normal, uv.x, uv.y), b0, b1, b2);

				const uint32_t pixelIndex = Memory::At(pathPool.pixels, hitData.path).index;
				
				float3 normal = shadingFrame.normal;
				const float4 radiance = contribution.throughput * make_float4(normal.x * 0.5f + 0.5f, normal.y * 0.5f + 0.5f, normal.z * 0.5f + 0.5f, 1.0f);

				atomicAdd(&Memory::At(accumulationBuffer, pixelIndex).x, radiance.x);
				atomicAdd(&Memory::At(accumulationBuffer, pixelIndex).y, radiance.y);
				atomicAdd(&Memory::At(accumulationBuffer, pixelIndex).z, radiance.z);
				pathTerminated = true;
				break;
			}
			case GlobalShadingModel::Diffuse:
			{
				Triangle triangle = Memory::At(triangles, hitData.triangle);
				
				float b0 = 1.0f - hitData.u - hitData.v;
				float b1 = hitData.u;
				float b2 = hitData.v;
				float2 uv =
					b0 * triangle.vertices[0].uv +
					b1 * triangle.vertices[1].uv +
					b2 * triangle.vertices[2].uv;

				Ray& ray = Memory::At(pathPool.rays, hitData.path);

				Math::Frame shadingFrame = GetShadingFrame(triangle, Memory::Sample(material.normal, uv.x, uv.y), b0, b1, b2);
				float4 albedo = Memory::Sample(material.color, uv.x, uv.y);
				Random& random = Memory::At(pathPool.randoms, hitData.path);
				float2 u = make_float2(curand_uniform(&random.state), curand_uniform(&random.state));

				float3 omegaI = shadingFrame.FromLocal(SampleCosineWeightedHemisphere(u));
																												  
				float3 geometricNormal = GetGeometricNormal(triangle);
				if (dot(ray.direction, geometricNormal) > 0.0f)
					geometricNormal = -geometricNormal;
				
				ray.origin = ray.origin + ray.direction * ray.tMax + geometricNormal * 1e-4f;
				ray.direction = omegaI;
				ray.tMin = PathTracerDefaults::MinT;
				ray.tMax = PathTracerDefaults::MaxT;
				contribution.throughput *= albedo;
				break;
			}
			case GlobalShadingModel::Mirror:
			{
				Triangle triangle = Memory::At(triangles, hitData.triangle);

				float b0 = 1.0f - hitData.u - hitData.v;
				float b1 = hitData.u;
				float b2 = hitData.v;
				float2 uv =
					b0 * triangle.vertices[0].uv +
					b1 * triangle.vertices[1].uv +
					b2 * triangle.vertices[2].uv;

				Ray& ray = Memory::At(pathPool.rays, hitData.path);
				Math::Frame shadingFrame = GetShadingFrame(triangle, Memory::Sample(material.normal, uv.x, uv.y), b0, b1, b2);
				
				float3 omegaI = reflect(ray.direction, shadingFrame.normal);
				float4 reflectance = Memory::Sample(material.specular, uv.x, uv.y);

				float3 geometricNormal = GetGeometricNormal(triangle);
				if (dot(ray.direction, geometricNormal) > 0.0f)
					geometricNormal = -geometricNormal;

				ray.origin = ray.origin + ray.direction * ray.tMax + geometricNormal * 1e-4f;
				ray.direction = omegaI;
				ray.tMin = PathTracerDefaults::MinT;
				ray.tMax = PathTracerDefaults::MaxT;
				contribution.throughput *= reflectance;
				break;
			}
			case GlobalShadingModel::Phong:
			{
				Triangle triangle = Memory::At(triangles, hitData.triangle);

				float b0 = 1.0f - hitData.u - hitData.v;
				float b1 = hitData.u;
				float b2 = hitData.v;

				float2 uv =
					b0 * triangle.vertices[0].uv +
					b1 * triangle.vertices[1].uv +
					b2 * triangle.vertices[2].uv;

				Ray& ray = Memory::At(pathPool.rays, hitData.path);

				float3 geometricNormal = GetGeometricNormal(triangle);
				if (dot(ray.direction, geometricNormal) > 0.0f)
					geometricNormal = -geometricNormal;

				Math::Frame shadingFrame = GetShadingFrame(triangle, Memory::Sample(material.normal, uv.x, uv.y), b0, b1, b2);

				float4 albedo = Memory::Sample(material.color, uv.x, uv.y);
				float4 specular = Memory::Sample(material.specular, uv.x, uv.y);

				float shininess = Memory::Sample(material.shininess, uv.x, uv.y) * (MaterialDefaults::MaxShininess - MaterialDefaults::MinShininess) + MaterialDefaults::MinShininess;
				float diffuseWeight = fmaxf(albedo.x, fmaxf(albedo.y, albedo.z));
				float specularWeight = fmaxf(specular.x, fmaxf(specular.y, specular.z));
				float weightSum = diffuseWeight + specularWeight;

				if (weightSum <= 0.0f)
				{
					pathTerminated = true;
					break;
				}

				float diffuseSelectionProbability = diffuseWeight / weightSum;
				float specularSelectionProbability = 1.0f - diffuseSelectionProbability;

				Random& random = Memory::At(pathPool.randoms, hitData.path);
				float3 reflectionDirection = normalize(reflect(ray.direction, shadingFrame.normal));
				float randomValue = curand_uniform(&random.state);

				float3 omegaI;
				float2 u = make_float2(curand_uniform(&random.state), curand_uniform(&random.state));

				if (randomValue < diffuseSelectionProbability)
				{
					omegaI = shadingFrame.FromLocal(SampleCosineWeightedHemisphere(u));
				}
				else
				{
					omegaI = SamplePhongLobe(reflectionDirection, shininess, u);
				}

				float cosThetaI = fmaxf(dot(shadingFrame.normal, omegaI), 0.0f);
				float geometricCosThetaI = fmaxf(dot(geometricNormal, omegaI), 0.0f);

				if (cosThetaI <= 0.0f || geometricCosThetaI <= 0.0f)
				{
					pathTerminated = true;
					break;
				}

				float cosThetaR = fmaxf(dot(reflectionDirection, omegaI), 0.0f);
				float phongLobeOverTwoPi = Math::InvTwoPi * powf(cosThetaR, shininess); 
				// this simplication can't be used in case of energy-normalized modified phong 
				// (in such case the normalization term is sampled from precomputed textures since it's not trivial to compute on the fly)

				float diffusePdf = cosThetaI * Math::InvPi;
				float specularPdf = (shininess + 1.0f) * phongLobeOverTwoPi;

				float pdf = diffuseSelectionProbability * diffusePdf + specularSelectionProbability * specularPdf;

				if (pdf <= 0.0f)
				{
					pathTerminated = true;
					break;
				}

				float4 diffuseBrdf = albedo * Math::InvPi;
				float4 specularBrdf = specular * (shininess + 2.0f) * phongLobeOverTwoPi;

				ray.origin = ray.origin + ray.direction * ray.tMax + geometricNormal * 1e-4f;
				ray.direction = omegaI;
				ray.tMin = PathTracerDefaults::MinT;
				ray.tMax = PathTracerDefaults::MaxT;

				contribution.throughput *= (diffuseBrdf + specularBrdf) * (cosThetaI / pdf);
				break;
			}
			case GlobalShadingModel::Ggx:
			{
				Triangle triangle = Memory::At(triangles, hitData.triangle);

				float b0 = 1.0f - hitData.u - hitData.v;
				float b1 = hitData.u;
				float b2 = hitData.v;

				float2 uv =
					b0 * triangle.vertices[0].uv +
					b1 * triangle.vertices[1].uv +
					b2 * triangle.vertices[2].uv;

				Ray& ray = Memory::At(pathPool.rays, hitData.path);

				float3 geometricNormal = GetGeometricNormal(triangle);
				if (dot(ray.direction, geometricNormal) > 0.0f)
					geometricNormal = -geometricNormal;
				
				Random& random = Memory::At(pathPool.randoms, hitData.path);

				Math::Frame shadingFrame = GetShadingFrame(triangle, Memory::Sample(material.normal, uv.x, uv.y), b0, b1, b2);
				float3 baseColor = make_float3(Memory::Sample(material.color, uv.x, uv.y));
				float4 rma = Memory::Sample(material.rma, uv.x, uv.y);

				float alpha = rma.x * rma.x;
				alpha = fmaxf(alpha, 0.001f); // in the future we should treat alpha=0 as a special case - effectively smooth surface
				float3 wo = shadingFrame.ToLocal(-ray.direction);

				if (wo.z <= 0.0f)
				{
					pathTerminated = true;
					break;
				}

				float dielectricF0 = Math::Sqr((ray.ior - material.ior) / (ray.ior + material.ior));
				float3 f0 = lerp(make_float3(dielectricF0), baseColor, rma.y);

				float3 diffuseColor = baseColor * (1.0f - rma.y);

				float diffuseWeight = luminance(diffuseColor);
				float specularWeight = luminance(f0);

				float weightSum = diffuseWeight + specularWeight;
				if (weightSum <= 1e-6f)
				{
					pathTerminated = true;
					break;
				}

				float diffuseSelectionProbability = diffuseWeight / weightSum;
				float specularSelectionProbability = specularWeight / weightSum;
				
				float lobeSample = curand_uniform(&random.state);
				float2 directionSample = make_float2(curand_uniform(&random.state), curand_uniform(&random.state));

				float3 wi;

				if (lobeSample < diffuseSelectionProbability)
				{
					wi = SampleCosineWeightedHemisphere(directionSample);
				}
				else
				{
					float3 sampledWm = TrowbridgeSampleWm(wo, make_float2(alpha, alpha), directionSample);
					wi = reflect(-wo, sampledWm);
				}

				if (wi.z <= 0.0f)
				{
					pathTerminated = true;
					break;
				}
				
				float3 wm = normalize(wi + wo);
				
				if (wm.z <= 0.0f)
				{
					pathTerminated = true;
					break;
				}

				float woDotWm = dot(wo, wm);
				
				if (woDotWm <= 1e-6f)
				{
					pathTerminated = true;
					break;
				}

				float cosThetaI = Math::CosTheta(wi);
				float cosThetaO = Math::CosTheta(wo);

				float3 fresnel = FresnelSchlick(f0, clamp(woDotWm, 0.0f, 1.0f));
				
				float3 diffuseBsdf = (1.0f - fresnel) * diffuseColor * Math::InvPi;
				float3 ggxBsdf = fresnel * D(wm, alpha) * G(wo, wi, alpha) / (4.0f * cosThetaI * cosThetaO);
			
				float diffusePdf = cosThetaI * Math::InvPi;
				float ggxPdf = D(wo, wm, alpha) / (4.0f * woDotWm);
				float pdf = diffuseSelectionProbability * diffusePdf + specularSelectionProbability * ggxPdf;
				
				if (pdf <= 1e-8f)
				{
					pathTerminated = true;
					break;
				}
				
				float3 worldWi = shadingFrame.FromLocal(wi);
				if (dot(worldWi, geometricNormal) <= 0.0f)
				{
					pathTerminated = true;
					break;
				}
				
				ray.origin = ray.origin + ray.direction * ray.tMax + geometricNormal * 1e-4f;
				ray.direction = worldWi;
				ray.tMin = PathTracerDefaults::MinT;
				ray.tMax = PathTracerDefaults::MaxT;

				contribution.throughput *= make_float4((diffuseBsdf + ggxBsdf) * (cosThetaI / pdf), 1.0f);
				break;
			}
			case GlobalShadingModel::Emissive:
			{
				Triangle triangle = Memory::At(triangles, hitData.triangle);
				
				float b0 = 1.0f - hitData.u - hitData.v;
				float b1 = hitData.u;
				float b2 = hitData.v;
				float2 uv =
					b0 * triangle.vertices[0].uv +
					b1 * triangle.vertices[1].uv +
					b2 * triangle.vertices[2].uv;
				
				const float4 emission = Memory::Sample(material.emission, uv.x, uv.y);
				const uint32_t pixelIndex = Memory::At(pathPool.pixels, hitData.path).index;
				const float4 radiance = contribution.throughput * emission;

				atomicAdd(&Memory::At(accumulationBuffer, pixelIndex).x, radiance.x);
				atomicAdd(&Memory::At(accumulationBuffer, pixelIndex).y, radiance.y);
				atomicAdd(&Memory::At(accumulationBuffer, pixelIndex).z, radiance.z);
				
				pathTerminated = true;
				break;
			}
		}
		
		PathFlags& pathFlags = Memory::At(pathPool.pathFlags, hitData.path);
		pathTerminated |= pathFlags.depth >= pathDepthLimit;
		pathFlags.depth++;

		if (!pathTerminated && pathFlags.depth >= PathTracerDefaults::RussianRouletteStartDepth)
		{
			float alpha = fmaxf(contribution.throughput.x, fmaxf(contribution.throughput.y, contribution.throughput.z));
			alpha = clamp(alpha, 0.0f, 1.0f);
			float u = curand_uniform(&Memory::At(pathPool.randoms, hitData.path).state);

			pathTerminated = alpha < u;

			if (!pathTerminated)
				Memory::At(pathPool.contributions, hitData.path).throughput = contribution.throughput / alpha;
		}

		if (pathTerminated)
		{
			Memory::Push(regenQueue, hitData.path);
		}
		else
		{
			Memory::Push(nextRayQueue, hitData.path);
		}
	}

	__global__ void RegeneratePathsKernel(
		uint32_t regenerateCount,
		Memory::DeviceQueueView<uint32_t> regenQueue, 
		uint64_t launchedSampleCount,
		DeviceCamera camera, 
		uint32_t width, 
		uint32_t height, 
		uint32_t sampleGridSize, 
		uint32_t spp,
		PathPoolView pathPool,
		Memory::DeviceQueueView<uint32_t> nextRayQueue)
	{
		const uint32_t queueIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (queueIndex >= regenerateCount) return;
		
		const uint32_t pathIndex = Memory::At(regenQueue, queueIndex);
		const uint64_t sampleIndex = launchedSampleCount + queueIndex;

		const uint32_t pixelIndex = static_cast<uint32_t>(sampleIndex / spp);
		const uint32_t sampleGridIndex = static_cast<uint32_t>(sampleIndex % spp);

		const uint32_t x = pixelIndex % width;
		const uint32_t y = pixelIndex / width;

		curandStatePhilox4_32_10_t state;

		curand_init(SEED, sampleIndex, 0ULL, &state);
		
		const uint32_t sampleGridX = sampleGridIndex % sampleGridSize;
		const uint32_t sampleGridY = sampleGridIndex / sampleGridSize;

		const float jitterX = curand_uniform(&state);
		const float jitterY = curand_uniform(&state);

		const float u = (x + (sampleGridX + jitterX) / sampleGridSize) / width;
		const float v = (y + (sampleGridY + jitterY) / sampleGridSize) / height;

		float3 rayDirection = normalize(camera.lowerLeftCorner + u * camera.horizontal + v * camera.vertical - camera.origin);

		Pixel& pixel = Memory::At(pathPool.pixels, pathIndex);
		pixel.index = pixelIndex;

		Ray& ray = Memory::At(pathPool.rays, pathIndex);
		ray.direction = rayDirection;
		ray.origin = camera.origin;
		ray.tMin = PathTracerDefaults::MinT;
		ray.tMax = PathTracerDefaults::MaxT;
		ray.ior = 1.0f;
		
		Contribution& contribution = Memory::At(pathPool.contributions, pathIndex);
		contribution.throughput = make_float4(1.0f);
		
		Random& random = Memory::At(pathPool.randoms, pathIndex);
		random.state = state;
		
		PathFlags& pathFlags = Memory::At(pathPool.pathFlags, pathIndex);
		pathFlags.depth = 0;

		Memory::At(nextRayQueue, *nextRayQueue.counter.value + queueIndex) = pathIndex;
	}

	__device__ __forceinline__ float4 TonemapReinhard(const float4& color)
	{
		return color / (color + make_float4(1.0f));
	}

	__device__ __forceinline__ float4 LinearToSrgb(const float4& color, float gamma)
	{
		float invGamma = 1.0f / gamma;
		return make_float4(powf(color.x, invGamma), powf(color.y, invGamma), powf(color.z, invGamma), color.w);
	}
	
	__global__ void PostprocessKernel(
		Memory::DeviceBuffer1DView<float4> accumulationBuffer,
		float invSpp,
		Memory::DeviceBuffer1DView<uchar4> framebuffer)
	{
		uint32_t pixelIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (pixelIndex >= framebuffer.size) return;

		const float4 hdrColor = Memory::At(accumulationBuffer, pixelIndex) * invSpp;
		const float4 ldrColor = LinearToSrgb(TonemapReinhard(hdrColor), 2.2f);
		Memory::At(framebuffer, pixelIndex) = make_uchar4(
			static_cast<unsigned char>(clamp(ldrColor.x * 255.0f, 0.0f, 255.0f)),
			static_cast<unsigned char>(clamp(ldrColor.y * 255.0f, 0.0f, 255.0f)),
			static_cast<unsigned char>(clamp(ldrColor.z * 255.0f, 0.0f, 255.0f)),
			255);
	}

	void Clear(uchar4 color, Memory::DeviceBuffer1DView<uchar4> framebuffer)
	{

		dim3 block(TPB_DIM_1D);
		dim3 grid(GetBlockCount(framebuffer.size, block.x));

		ClearKernel<<<grid, block>>>(color, framebuffer);
	}

	void InitializePaths(
		DeviceCamera camera, 
		uint32_t width, 
		uint32_t height, 
		uint32_t sampleGridSize, 
		uint32_t generateCount, 
		PathPoolView pathPool, 
		Memory::DeviceQueueView<uint32_t> rayQueue)
	{
		uint32_t spp = sampleGridSize * sampleGridSize;
		dim3 block(TPB_DIM_1D);
		dim3 grid(GetBlockCount(generateCount, block.x));

		InitializePathsKernel<<<grid, block>>>(
			camera,
			width,
			height,
			sampleGridSize,
			spp,
			generateCount,
			pathPool,
			rayQueue);
	}

	void IntersectRaysWithScene(
		uint32_t activePaths, 
		PathPoolView pathPool, 
		Memory::DeviceQueueView<uint32_t> rayQueue, 
		DeviceBvhView bvh, 
		Memory::DeviceQueueView<HitData> hitQueue,
		Memory::DeviceQueueView<uint32_t> regenQueue)
	{
		dim3 block(TPB_DIM_1D);
		dim3 grid(GetBlockCount(activePaths, block.x));
		IntersectRaysWithSceneKernel<<<grid, block>>>(pathPool, rayQueue, bvh, hitQueue, regenQueue);
	}

	void ResolveHits(
		uint32_t hitCount,
		PathPoolView pathPool, 
		Memory::DeviceQueueView<HitData> hitQueue, 
		Memory::DeviceBuffer1DView<Triangle> triangles,
		Memory::DeviceBuffer1DView<Material> materials,
		uint32_t pathDepthLimit,
		Memory::DeviceQueueView<uint32_t> regenQueue,
		Memory::DeviceBuffer1DView<float4> accumulationBuffer,
		Memory::DeviceQueueView<uint32_t> nextRayQueue)
	{
		if (hitCount <= 0) return;
		dim3 block(TPB_DIM_1D);
		dim3 grid(GetBlockCount(hitCount, block.x));
		ResolveHitsKernel<<<grid, block>>>(pathPool, hitQueue, triangles, materials, pathDepthLimit, regenQueue, accumulationBuffer, nextRayQueue);
	}

	void RegeneratePaths(
		uint32_t regenerateCount,
		Memory::DeviceQueueView<uint32_t> regenQueue,
		uint64_t launchedSampleCount,
		DeviceCamera camera, 
		uint32_t width, 
		uint32_t height, 
		uint32_t sampleGridSize,
		PathPoolView pathPool,
		Memory::DeviceQueueView<uint32_t> nextRayQueue)
	{
		if (regenerateCount <= 0) return;
		uint32_t spp = sampleGridSize * sampleGridSize;
		dim3 block(TPB_DIM_1D);
		dim3 grid(GetBlockCount(regenerateCount, block.x));
		RegeneratePathsKernel<<<grid, block>>>(regenerateCount, regenQueue, launchedSampleCount, camera, width, height, sampleGridSize, spp, pathPool, nextRayQueue);
	}

	void PostprocessAccumulatedRadiance(
		Memory::DeviceBuffer1DView<float4> accumulationBuffer,
		float invSpp,
		Memory::DeviceBuffer1DView<uchar4> framebuffer)
	{
		dim3 block(TPB_DIM_1D);
		dim3 grid(GetBlockCount(accumulationBuffer.size, block.x));

		PostprocessKernel<<<grid, block>>>(accumulationBuffer, invSpp, framebuffer);
	}
}
