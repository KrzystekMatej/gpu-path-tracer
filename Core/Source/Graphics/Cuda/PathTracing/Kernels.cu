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

	__device__ __forceinline__ float3 GetInterpolatedNormal(const Triangle& triangle, float b0, float b1, float b2)
	{
		return normalize(
			b0 * triangle.vertices[0].normal +
			b1 * triangle.vertices[1].normal +
			b2 * triangle.vertices[2].normal
		);
	}

	__device__ __forceinline__ float3 GetShadingNormal(const Triangle& triangle, float4 normalMapSample, float b0, float b1, float b2)
	{
		float2 uv =
			b0 * triangle.vertices[0].uv +
			b1 * triangle.vertices[1].uv +
			b2 * triangle.vertices[2].uv;

		float3 N = normalize(
			b0 * triangle.vertices[0].normal +
			b1 * triangle.vertices[1].normal +
			b2 * triangle.vertices[2].normal
		);

		float4 interpolatedTangent =
			b0 * triangle.vertices[0].tangent +
			b1 * triangle.vertices[1].tangent +
			b2 * triangle.vertices[2].tangent;

		float3 T = make_float3(
			interpolatedTangent.x,
			interpolatedTangent.y,
			interpolatedTangent.z
		);

		T = normalize(T - N * dot(N, T));

		float tangentSign = interpolatedTangent.w < 0.0f ? -1.0f : 1.0f;
		float3 B = normalize(cross(N, T)) * tangentSign;

		float3 tangentSpaceNormal = normalize(make_float3(
			normalMapSample.x * 2.0f - 1.0f,
			normalMapSample.y * 2.0f - 1.0f,
			normalMapSample.z * 2.0f - 1.0f
		));

		return normalize(
			tangentSpaceNormal.x * T +
			tangentSpaceNormal.y * B +
			tangentSpaceNormal.z * N
		);
	}

	__device__ __forceinline__ float3 SampleCosineWeightedHemisphere(float3 normal, curandStatePhilox4_32_10_t& state)
	{
		float u1 = curand_uniform(&state);
		float u2 = curand_uniform(&state);

		float phi = 2.0f * CUDART_PI_F * u1;
		float cosTheta = sqrtf(u2);
		float sinTheta = sqrtf(1.0f - u2);

		float3 tangent;
		if (fabsf(normal.x) > fabsf(normal.z))
		{
			tangent = normalize(make_float3(normal.y, -normal.x, 0.0f)); // cross shortcut - n x (0, 0, 1) = ( n.y, -n.x, 0 )
		}
		else
		{
			tangent = normalize(make_float3(0.0f, normal.z, -normal.y)); // cross shortcut - n x (1, 0, 0) = ( 0, n.z, -n.y )
		}

		float3 bitangent = cross(normal, tangent);

		return normalize(
			cosTheta * normal +
			sinTheta * cosf(phi) * tangent +
			sinTheta * sinf(phi) * bitangent
		);
	}

	__device__ __forceinline__ float3 SamplePhongLobe(float3 axis, float exponent, curandStatePhilox4_32_10_t& state)
	{
		float u1 = curand_uniform(&state);
		float u2 = curand_uniform(&state);
		
		float phi = 2.0f * CUDART_PI_F * u1;
		float cosTheta = powf(u2, 1.0f / (exponent + 1.0f));
		float sinTheta = sqrtf(1.0f - cosTheta * cosTheta);

		float3 tangent;
		if (fabsf(axis.x) > fabsf(axis.z))
		{
			tangent = normalize(make_float3(axis.y, -axis.x, 0.0f)); // cross shortcut - n x (0, 0, 1) = ( n.y, -n.x, 0 )
		}
		else
		{
			tangent = normalize(make_float3(0.0f, axis.z, -axis.y)); // cross shortcut - n x (1, 0, 0) = ( 0, n.z, -n.y )
		}

		float3 bitangent = cross(axis, tangent);
		return normalize(
			cosTheta * axis +
			sinTheta * cosf(phi) * tangent +
			sinTheta * sinf(phi) * bitangent
		);
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

				float3 normal = GetShadingNormal(
						triangle,
						Memory::Sample(material.normal, uv.x, uv.y),
						b0,
						b1,
						b2);

				const uint32_t pixelIndex = Memory::At(pathPool.pixels, hitData.path).index;
				
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

				float3 normal = GetShadingNormal(
						triangle,
						Memory::Sample(material.normal, uv.x, uv.y),
						b0,
						b1,
						b2);

				float4 albedo = Memory::Sample(material.albedo, uv.x, uv.y);
				float3 omegaI = SampleCosineWeightedHemisphere(normal, Memory::At(pathPool.randoms, hitData.path).state);
																												  
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
				float3 normal = GetShadingNormal(
						triangle,
						Memory::Sample(material.normal, uv.x, uv.y),
						b0,
						b1,
						b2);
				
				float3 omegaI = reflect(ray.direction, normal);
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

				float3 normal = GetShadingNormal(
					triangle,
					Memory::Sample(material.normal, uv.x, uv.y),
					b0,
					b1,
					b2);

				if (dot(normal, geometricNormal) < 0.0f)
					normal = -normal;

				float4 albedo = Memory::Sample(material.albedo, uv.x, uv.y);
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

				curandStatePhilox4_32_10_t& randomState = Memory::At(pathPool.randoms, hitData.path).state;

				float3 reflectionDirection = normalize(reflect(ray.direction, normal));
				float randomValue = curand_uniform(&randomState);

				float3 omegaI;
				if (randomValue < diffuseSelectionProbability)
				{
					omegaI = SampleCosineWeightedHemisphere(normal, randomState);
				}
				else
				{
					omegaI = SamplePhongLobe(reflectionDirection, shininess, randomState);
				}

				float cosThetaI = fmaxf(dot(normal, omegaI), 0.0f);
				float geometricCosThetaI = fmaxf(dot(geometricNormal, omegaI), 0.0f);
				float cosThetaR = fmaxf(dot(reflectionDirection, omegaI), 0.0f);

				if (cosThetaI <= 0.0f || geometricCosThetaI <= 0.0f)
				{
					pathTerminated = true;
					break;
				}

				float diffusePdf = cosThetaI / CUDART_PI_F;
				float specularPdf = ((shininess + 1.0f) / (2.0f * CUDART_PI_F)) * powf(cosThetaR, shininess);

				float pdf =
					diffuseSelectionProbability * diffusePdf +
					specularSelectionProbability * specularPdf;

				if (pdf <= 0.0f)
				{
					pathTerminated = true;
					break;
				}

				float4 diffuseBrdf = albedo / CUDART_PI_F;
				float4 specularBrdf = specular * ((shininess + 2.0f) / (2.0f * CUDART_PI_F)) * powf(cosThetaR, shininess);

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

				float3 normal = GetShadingNormal(
					triangle,
					Memory::Sample(material.normal, uv.x, uv.y),
					b0,
					b1,
					b2);

				if (dot(normal, geometricNormal) < 0.0f)
					normal = -normal;

				
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
