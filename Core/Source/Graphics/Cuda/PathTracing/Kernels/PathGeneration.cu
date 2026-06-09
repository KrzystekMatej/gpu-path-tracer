#include <Core/Graphics/Cuda/PathTracing/Kernels/LaunchUtils.cuh>
#include <Core/Graphics/Cuda/PathTracing/Kernels/Launchers.hpp>
#include <Core/Graphics/Cuda/PathTracing/PathTracerDefaults.hpp>
#include <Core/Graphics/Cuda/Utils/Math.hpp>
#include <cassert>

namespace Core::Graphics::Cuda::Kernels
{
	__global__ void InitializePathsKernel(
		uint32_t generateCount,
		DeviceCamera camera, 
		uint32_t width, 
		uint32_t height, 
		uint32_t sampleGridSize, 
		uint32_t spp, 
		PathPoolView pathPool, 
		RayQueueView rayQueue)
	{
		const uint32_t sampleIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (sampleIndex >= generateCount) return;

		const uint32_t pixelIndex = sampleIndex / spp;
		const uint32_t sampleGridIndex = sampleIndex % spp;

		const uint32_t x = pixelIndex % width;
		const uint32_t y = pixelIndex / width;

		
		const uint32_t sampleGridX = sampleGridIndex % sampleGridSize;
		const uint32_t sampleGridY = sampleGridIndex / sampleGridSize;

        Random random(PathTracerDefaults::RandomSeed, sampleIndex);
        
        const float2 jitter = random.NextFloat2();

		const float u = (x + (sampleGridX + jitter.x) / sampleGridSize) / width;
		const float v = (y + (sampleGridY + jitter.y) / sampleGridSize) / height;

		float3 rayDirection = normalize(camera.lowerLeftCorner + u * camera.horizontal + v * camera.vertical - camera.origin);
		
		// sampleIndex == pathIndex in this kernel
        pathPool.pixels.At(sampleIndex).index = pixelIndex;
        pathPool.randoms.At(sampleIndex) = random;

		Path path{ sampleIndex };
		Ray ray;
		ray.direction = rayDirection;
		ray.origin = camera.origin;
		ray.tMin = PathTracerDefaults::MinT;
		ray.tMax = PathTracerDefaults::MaxT;
		ray.ior = 1.0f;
		ray.depth = 0;
		ray.throughput = make_float3(1.0f);
		rayQueue.Set(sampleIndex, path, ray);
	}
    
	__global__ void RegeneratePathsKernel(
		uint32_t remainingCount,
		RegenQueueView regenQueue, 
		uint64_t launchedSampleCount,
		DeviceCamera camera, 
		uint32_t width, 
		uint32_t height, 
		uint32_t sampleGridSize, 
		uint32_t spp,
		PathPoolView pathPool,
		RayQueueView nextRayQueue)
	{
		const uint32_t queueIndex = blockIdx.x * blockDim.x + threadIdx.x;
		uint32_t regenerateCount = Math::Min(remainingCount, regenQueue.GetSize());
		if (queueIndex >= regenerateCount) return;
		
		Path path = regenQueue.GetPath(queueIndex);
		const uint64_t sampleIndex = launchedSampleCount + queueIndex;

		const uint32_t pixelIndex = static_cast<uint32_t>(sampleIndex / spp);
		const uint32_t sampleGridIndex = static_cast<uint32_t>(sampleIndex % spp);

		const uint32_t x = pixelIndex % width;
		const uint32_t y = pixelIndex / width;

        Random random(PathTracerDefaults::RandomSeed, sampleIndex);
		
		const uint32_t sampleGridX = sampleGridIndex % sampleGridSize;
		const uint32_t sampleGridY = sampleGridIndex / sampleGridSize;

		const float2 jitter = random.NextFloat2();

		const float u = (x + (sampleGridX + jitter.x) / sampleGridSize) / width;
		const float v = (y + (sampleGridY + jitter.y) / sampleGridSize) / height;

		float3 rayDirection = normalize(camera.lowerLeftCorner + u * camera.horizontal + v * camera.vertical - camera.origin);

        pathPool.pixels.At(path.index).index = pixelIndex;
        pathPool.randoms.At(path.index) = random;

		Ray ray;
		ray.direction = rayDirection;
		ray.origin = camera.origin;
		ray.tMin = PathTracerDefaults::MinT;
		ray.tMax = PathTracerDefaults::MaxT;
		ray.ior = 1.0f;
		ray.depth = 0;
		ray.throughput = make_float3(1.0f);
		nextRayQueue.Set(nextRayQueue.GetSize() + queueIndex, path, ray);
	}
    
	void InitializePaths(
		cudaStream_t stream,
		uint32_t generateCount,
		DeviceCamera camera, 
		uint32_t width, 
		uint32_t height, 
		uint32_t sampleGridSize, 
		PathPoolView pathPool, 
		RayQueueView rayQueue)
	{
		assert(generateCount > 0);
		uint32_t spp = sampleGridSize * sampleGridSize;
		dim3 block(LaunchUtils::InitializeThreadsPerBlock);
		dim3 grid(LaunchUtils::GetBlockCount(generateCount, block.x));

		InitializePathsKernel<<<grid, block, 0, stream>>>(
			generateCount,
			camera,
			width,
			height,
			sampleGridSize,
			spp,
			pathPool,
			rayQueue);
	}
    
	void RegeneratePaths(
		cudaStream_t stream,
		uint32_t remainingCount,
		RegenQueueView regenQueue,
		uint64_t launchedSampleCount,
		DeviceCamera camera, 
		uint32_t width, 
		uint32_t height, 
		uint32_t sampleGridSize,
		PathPoolView pathPool,
		RayQueueView nextRayQueue)
	{
		if (remainingCount == 0) return;
		uint32_t spp = sampleGridSize * sampleGridSize;
		dim3 block(LaunchUtils::RegenerateThreadsPerBlock);
		dim3 grid(LaunchUtils::GetBlockCount(std::min(remainingCount, regenQueue.GetCapacity()), block.x));
		RegeneratePathsKernel<<<grid, block, 0, stream>>>(remainingCount, regenQueue, launchedSampleCount, camera, width, height, sampleGridSize, spp, pathPool, nextRayQueue);
	}
}