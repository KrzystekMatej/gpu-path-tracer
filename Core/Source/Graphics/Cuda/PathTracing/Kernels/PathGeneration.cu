#include <Core/Graphics/Cuda/PathTracing/Kernels/LaunchUtils.cuh>
#include <Core/Graphics/Cuda/PathTracing/Kernels/Launchers.hpp>
#include <Core/Graphics/Cuda/PathTracing/PathTracerDefaults.hpp>
#include <Core/Graphics/Cuda/Utils/Math.hpp>
#include <cassert>

namespace Core::Graphics::Cuda::Kernels
{
	struct PixelSample
	{
		uint32_t pixel;
		float2 uv;
	};
	
	__device__ __forceinline__ PixelSample ComputePathSample(const PixelGrid& grid, uint64_t sampleIndex, float2 jitter)
	{
		Math::DivisionResult<uint64_t> sampleDiv = Math::DivideWithRemainder(sampleIndex, static_cast<uint64_t>(grid.samplesPerPixel));
		const uint32_t pixelIndex = static_cast<uint32_t>(sampleDiv.quotient);
		const uint32_t sampleGridIndex = static_cast<uint32_t>(sampleDiv.remainder);

		Math::DivisionResult<uint32_t> pixelDiv = Math::DivideWithRemainder(pixelIndex, grid.width);
		const uint32_t x = pixelDiv.remainder;
		const uint32_t y = pixelDiv.quotient;

		Math::DivisionResult<uint32_t> sampleGridDiv = Math::DivideWithRemainder(sampleGridIndex, grid.samplesPerPixelAxis);
		const uint32_t sampleGridX = sampleGridDiv.remainder;
		const uint32_t sampleGridY = sampleGridDiv.quotient;

		const float u = (x + (sampleGridX + jitter.x) / grid.samplesPerPixelAxis) / grid.width;
		const float v = (y + (sampleGridY + jitter.y) / grid.samplesPerPixelAxis) / grid.height;

		return PixelSample{ pixelIndex, make_float2(u, v) };
	}

	__device__ __forceinline__ Ray GenerateCameraRay(const DeviceCamera& camera, float2 uv)
	{
		Ray ray;
		ray.direction = normalize(camera.lowerLeftCorner + uv.x * camera.horizontal + uv.y * camera.vertical - camera.origin);
		ray.origin = camera.origin;
		ray.tMin = PathTracerDefaults::MinT;
		ray.tMax = PathTracerDefaults::MaxT;
		return ray;
	}
	
	__device__ __forceinline__ Path GeneratePath(uint32_t index)
	{
		Path path{};
		path.index = index;
		path.depth = 0;
		path.throughput = make_float3(1.0f, 1.0f, 1.0f);
		path.currentMediumIor = MaterialDefaults::DefaultIor;
		path.lastScatterWasDelta = true; // if the first bounce is emissive, we want to allow contribution
		return path;
	}

	__global__ void InitializePathsKernel(
		uint32_t generateCount,
		DeviceCamera camera, 
		PixelGrid pixelGrid,
		PathPoolView pathPool, 
		RayQueueView rayQueue)
	{
		const uint32_t sampleIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (sampleIndex >= generateCount) return;

        Random random(PathTracerDefaults::RandomSeed, sampleIndex);
        const float2 jitter = random.NextFloat2();

		const PixelSample sample = ComputePathSample(pixelGrid, sampleIndex, jitter);
		
		// sampleIndex == pathIndex in this kernel
        pathPool.pixels.At(sampleIndex) = Pixel{ sample.pixel };
        pathPool.randoms.At(sampleIndex) = random;

		rayQueue.Set(sampleIndex, GeneratePath(sampleIndex), GenerateCameraRay(camera, sample.uv));
	}
    
	__global__ void RegeneratePathsKernel(
		uint32_t remainingCount,
		RegenQueueView regenQueue, 
		uint64_t launchedSampleCount,
		DeviceCamera camera, 
		PixelGrid pixelGrid,
		PathPoolView pathPool,
		RayQueueView nextRayQueue)
	{
		const uint32_t queueIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (queueIndex >= Math::Min(remainingCount, regenQueue.GetSize())) return;
		
		uint32_t pathIndex = regenQueue.GetPathIndex(queueIndex);
		const uint64_t sampleIndex = launchedSampleCount + queueIndex;

        Random random(PathTracerDefaults::RandomSeed, sampleIndex);
		const float2 jitter = random.NextFloat2();

		const PixelSample sample = ComputePathSample(pixelGrid, sampleIndex, jitter);

        pathPool.pixels.At(pathIndex) = Pixel{ sample.pixel };
        pathPool.randoms.At(pathIndex) = random;

		nextRayQueue.Set(nextRayQueue.GetSize() + queueIndex, GeneratePath(pathIndex), GenerateCameraRay(camera, sample.uv));
	}
    
	void InitializePaths(
		cudaStream_t stream,
		uint32_t generateCount,
		DeviceCamera camera, 
		PixelGrid pixelGrid,
		PathPoolView pathPool, 
		RayQueueView rayQueue)
	{
		assert(generateCount > 0);
		dim3 block(LaunchUtils::InitializeThreadsPerBlock);
		dim3 grid(LaunchUtils::GetBlockCount(generateCount, block.x));

		InitializePathsKernel<<<grid, block, 0, stream>>>(
			generateCount,
			camera,
			pixelGrid,
			pathPool,
			rayQueue);
	}
    
	void RegeneratePaths(
		cudaStream_t stream,
		uint32_t remainingCount,
		RegenQueueView regenQueue,
		uint64_t launchedSampleCount,
		DeviceCamera camera, 
		PixelGrid pixelGrid,
		PathPoolView pathPool,
		RayQueueView nextRayQueue)
	{
		if (remainingCount == 0) return;
		dim3 block(LaunchUtils::RegenerateThreadsPerBlock);
		dim3 grid(LaunchUtils::GetBlockCount(std::min(remainingCount, regenQueue.GetCapacity()), block.x));
		RegeneratePathsKernel<<<grid, block, 0, stream>>>(remainingCount, regenQueue, launchedSampleCount, camera, pixelGrid, pathPool, nextRayQueue);
	}
}