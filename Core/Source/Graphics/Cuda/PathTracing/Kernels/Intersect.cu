#include <Core/Graphics/Cuda/PathTracing/Kernels/LaunchUtils.cuh>
#include <Core/Graphics/Cuda/PathTracing/Kernels/Launchers.hpp>
#include <Core/Graphics/Cuda/PathTracing/PathTracerDefaults.hpp>
#include <Core/Graphics/Cuda/Bvh/BvhDefaults.hpp>
#include <Core/Graphics/Cuda/Runtime/Memory/Stack.hpp>
#include <cassert>
#include <Core/Graphics/Cuda/Utils/Math.hpp>

namespace Core::Graphics::Cuda::Kernels
{
	__device__ __forceinline__ bool AabbIntersect(const BoundingBox& bounds, const Ray& ray, float3 invDirection)
	{
		const float3 t0 = (bounds.min - ray.origin) * invDirection;
		const float3 t1 = (bounds.max - ray.origin) * invDirection;

		const float3 lo = fminf(t0, t1);
		const float3 hi = fmaxf(t0, t1);

		const float tNear = fmaxf(ray.tMin, Math::MaxComponent(lo));
		const float tFar = fminf(ray.tMax, Math::MinComponent(hi));

		return tNear <= tFar;
	}

	__device__ __forceinline__ bool IntersectTriangle(
		const TriangleIntersection& intersectionData,
		const Ray& ray,
		float& t,
		float& u,
		float& v)
	{
		float3 edge1 = make_float3(intersectionData.edge1);
		float3 edge2 = make_float3(intersectionData.edge2);
		float3 v0 = intersectionData.v0;

		const float3 p = cross(ray.direction, edge2);

		const float det = dot(edge1, p);
		const float epsilon = 1e-7f;

		if (fabs(det) < epsilon)
			return false;

		const float invDet = 1.0f / det;

		const float3 s = ray.origin - v0;
		u = dot(s, p) * invDet;

		if (u < 0.0f || u > 1.0f)
			return false;

		const float3 q = cross(s, edge1);
		v = dot(ray.direction, q) * invDet;

		if (v < 0.0f || u + v > 1.0f)
			return false;

		t = dot(edge2, q) * invDet;

		return t >= ray.tMin && t < ray.tMax;
	}
    
	template<uint32_t BvhDepthUpperBound>
	__global__ void IntersectRaysWithSceneKernel(
		PathPoolView pathPool, 
		RayQueueView rayQueue,
		IntersectionBvhView bvh,
		MaterialEvalQueueViewsProvider materialQueueProvider,
		RegenQueueView regenQueue)
	{
		const uint32_t queueIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (queueIndex >= rayQueue.GetSize()) return;
		
		const Path path = rayQueue.GetPath(queueIndex);
		
		Ray ray = rayQueue.GetRay(queueIndex);
		HitData closestHit = { PathTracerDefaults::InvalidIndex, 0.0f, 0.0f };
		GlobalShadingModel shadingModel = GlobalShadingModel::Count;

		const float3 invDirection = make_float3(1.0f) / ray.direction;

		Runtime::Stack<uint32_t, BvhDepthUpperBound + 2> stack;
		stack.Push(0);
		
		while (!stack.IsEmpty())
		{
			const DeviceBvhNode node = bvh.GetNode(stack.Pop());

			if (!AabbIntersect(node.bounds, ray, invDirection))
				continue;

			if (node.IsLeaf())
			{
				for (uint32_t i = 0; i < node.count; i++)
				{
					const TriangleIntersection triangle = bvh.GetTriangle(node.first + i);
					float t, u, v;
					if (IntersectTriangle(triangle, ray, t, u, v))
					{
						ray.tMax = t;
						
						closestHit.triangle = node.first + i;
						closestHit.u = u;
						closestHit.v = v;
						shadingModel = triangle.shadingModel;
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
			materialQueueProvider.At(static_cast<uint32_t>(shadingModel)).Push(path, ray, closestHit);
		}
		else
		{
			regenQueue.Push(path);
		}
	}
    
	template <int Depth, int... RemainingDepths>
	void LaunchIntersectKernel(
		int bvhDepth,
		dim3 grid,
		dim3 block,
		cudaStream_t stream,
		PathPoolView pathPool,
		RayQueueView rayQueue,
		IntersectionBvhView bvh,
		MaterialEvalQueueViewsProvider materialQueueProvider,
		RegenQueueView regenQueue)
	{
		if (bvhDepth <= Depth)
		{
			IntersectRaysWithSceneKernel<Depth><<<grid, block, 0, stream>>>(
				pathPool,
				rayQueue,
				bvh,
				materialQueueProvider,
				regenQueue);
			return;
		}

		if constexpr (sizeof...(RemainingDepths) > 0)
		{
			LaunchIntersectKernel<RemainingDepths...>(
				bvhDepth,
				grid,
				block,
				stream,
				pathPool,
				rayQueue,
				bvh,
				materialQueueProvider,
				regenQueue);
		}
	}

	void IntersectRaysWithScene(
		cudaStream_t stream,
		uint32_t queueSize,
		PathPoolView pathPool, 
		RayQueueView rayQueue, 
		IntersectionBvhView bvh,
		uint32_t bvhDepth,
		MaterialEvalQueueViewsProvider materialQueueProvider,
		RegenQueueView regenQueue)
	{
		assert(queueSize > 0);
		dim3 block(LaunchUtils::IntersectThreadsPerBlock);
		dim3 grid(LaunchUtils::GetBlockCount(queueSize, block.x));

		LaunchIntersectKernel<2, 4, 8, 12, 16, 24, 32, 40, 48, 56, BvhDefaults::MaxDepth>(
			bvhDepth,
			grid,
			block,
			stream,
			pathPool,
			rayQueue,
			bvh,
			materialQueueProvider,
			regenQueue);
	}
}