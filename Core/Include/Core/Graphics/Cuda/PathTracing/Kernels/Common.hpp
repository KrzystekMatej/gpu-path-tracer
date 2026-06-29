#pragma once
#include <cuda_runtime.h>
#include <Core/Graphics/Cuda/Utils/Math.hpp>
#include <Core/Graphics/Cuda/PathTracing/Memory/PathPoolView.hpp>

namespace Core::Graphics::Cuda::Kernels
{
	__device__ __forceinline__ void AddRadiance(PathPoolView pathPool, uint32_t pathIndex, float3 radiance, Runtime::DeviceBuffer1DView<float4> accumulationBuffer)
	{
		const Pixel pixel = pathPool.pixels.At(pathIndex);
#if defined(__CUDA_ARCH__)
		atomicAdd(&accumulationBuffer.At(pixel.index).x, radiance.x);
		atomicAdd(&accumulationBuffer.At(pixel.index).y, radiance.y);
		atomicAdd(&accumulationBuffer.At(pixel.index).z, radiance.z);
#endif
    }
}