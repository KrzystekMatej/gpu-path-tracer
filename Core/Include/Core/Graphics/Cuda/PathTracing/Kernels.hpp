#pragma once
#include <cuda_runtime.h>
#include <Core/Graphics/Cuda/Memory/DeviceBuffer1DView.hpp>
#include <Core/Graphics/Cuda/Memory/DeviceBuffer2DView.hpp>
#include <Core/Graphics/Cuda/PathTracing/DeviceCamera.hpp>
#include <Core/Graphics/Cuda/PathTracing/PathPoolView.hpp>
#include <Core/Graphics/Cuda/Memory/DeviceQueueView.hpp>
#include <Core/Graphics/Cuda/Bvh/DeviceBvhView.hpp>
#include <Core/Graphics/Cuda/PathTracing/Material.hpp>

namespace Core::Graphics::Cuda::Kernels
{
	void Clear(uchar4 color, Memory::DeviceBuffer1DView<uchar4> framebuffer);
	void SetCounters(
		Memory::DeviceQueueView<uint32_t> nextRayQueue,
		Memory::DeviceQueueView<HitData> hitQueue, 
		Memory::DeviceQueueView<uint32_t> regenQueue, 
		uint64_t totalSamples,
		Memory::CounterView<uint64_t> launchedSampleCounter);
	void InitializePaths(
		DeviceCamera camera, 
		uint32_t width, 
		uint32_t height, 
		uint32_t sampleGridSize, 
		uint32_t generateCount, 
		PathPoolView pathPool, 
		Memory::DeviceQueueView<uint32_t> rayQueue);
	void IntersectRaysWithScene(
		PathPoolView pathPool, 
		Memory::DeviceQueueView<uint32_t> rayQueue, 
		DeviceBvhView bvh,
		Memory::DeviceQueueView<HitData> hitQueue,
		Memory::DeviceQueueView<uint32_t> regenQueue);
	void ResolveHits(
		PathPoolView pathPool, 
		Memory::DeviceQueueView<HitData> hitQueue, 
		Memory::DeviceBuffer1DView<Triangle> triangles, 
		Memory::DeviceBuffer1DView<Material> materials,
		uint32_t pathDepthLimit,
		Memory::DeviceQueueView<uint32_t> regenQueue,
		Memory::DeviceBuffer1DView<float4> accumulationBuffer,
		Memory::DeviceQueueView<uint32_t> nextRayQueue);
	void RegeneratePaths(
		Memory::DeviceQueueView<uint32_t> regenQueue,
		uint64_t totalSamples,
		Memory::CounterView<uint64_t> launchedSampleCounter,
		DeviceCamera camera, 
		uint32_t width, 
		uint32_t height, 
		uint32_t sampleGridSize, 
		PathPoolView pathPool,
		Memory::DeviceQueueView<uint32_t> nextRayQueue);

	void PostprocessAccumulatedRadiance(
		Memory::DeviceBuffer1DView<float4> accumulationBuffer,
		float invSpp,
		Memory::DeviceBuffer1DView<uchar4> framebuffer);
}