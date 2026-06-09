#include <Core/Graphics/Cuda/PathTracing/Kernels/LaunchUtils.cuh>
#include <Core/Graphics/Cuda/PathTracing/Kernels/Launchers.hpp>
#include <Core/Graphics/Cuda/PathTracing/PathTracerDefaults.hpp>
#include <Core/Graphics/Cuda/Bvh/BvhDefaults.hpp>
#include <Core/Graphics/Cuda/Runtime/Memory/Stack.hpp>
#include <cassert>

namespace Core::Graphics::Cuda::Kernels
{
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
    
	template<uint32_t BvhDepthUpperBound>
	__global__ void IntersectRaysWithSceneKernel(
		PathPoolView pathPool, 
		RayQueueView rayQueue,
		DeviceBvhView bvh, 
		MaterialEvalQueueViewsProvider materialQueueProvider,
		RegenQueueView regenQueue)
	{
		const uint32_t queueIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (queueIndex >= rayQueue.GetSize()) return;
		
		const Path path = rayQueue.GetPath(queueIndex);
		Ray ray = rayQueue.GetRay(queueIndex);
		const float3 invDirection = make_float3(1.0f) / ray.direction;
		Runtime::Stack<uint32_t, BvhDepthUpperBound + 2> stack;
		stack.Push(0);
		HitData closestHit;
		closestHit.triangle = PathTracerDefaults::InvalidIndex;
		float closestT = ray.tMax;
		GlobalShadingModel shadingModel = GlobalShadingModel::Count;

		while (!stack.IsEmpty())
		{
			uint32_t nodeIndex = stack.Pop();
			const DeviceBvhNode node = bvh.nodes.At(nodeIndex);

			if (!AabbIntersect(node.bounds, ray.origin, invDirection, ray.tMin, closestT))
				continue;

			if (node.left == PathTracerDefaults::InvalidIndex && node.right == PathTracerDefaults::InvalidIndex)
			{
				for (uint32_t i = 0; i < node.count; i++)
				{
					const Triangle triangle = bvh.triangles.At(node.first + i);
					float t, u, v;
					if (IntersectTriangle(triangle, ray.origin, ray.direction, ray.tMin, closestT, t, u, v) && t < closestT)
					{
						closestT = t;
						
						closestHit.triangle = node.first + i;
						closestHit.material = triangle.materialIndex;
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
			ray.tMax = closestT;
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
		DeviceBvhView bvh,
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
		DeviceBvhView bvh,
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