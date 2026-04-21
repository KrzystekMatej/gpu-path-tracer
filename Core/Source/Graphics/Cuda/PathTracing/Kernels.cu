#include <Core/Graphics/Cuda/PathTracing/Kernels.hpp>
#include <Core/Graphics/Cuda/Memory/ViewUtils.cuh>
#include <cuda_runtime.h>
#include <cstdio>

namespace Core::Graphics::Cuda::Kernels
{
	#define TPB_1D 256

	__global__ void ClearKernel(uchar4 color, Memory::DeviceBuffer2DView<uchar4> framebuffer)
	{
		uint32_t tx = blockIdx.x * blockDim.x + threadIdx.x;
		uint32_t ty = blockIdx.y * blockDim.y + threadIdx.y;

		if (tx >= framebuffer.width || ty >= framebuffer.height) return;

		Memory::At(framebuffer, ty, tx) = color;
	}

	void Clear(uchar4 color, Memory::DeviceBuffer2DView<uchar4> framebuffer)
	{

		dim3 block = { TPB_1D, TPB_1D, 1 };
		dim3 grid
		{ 
			(static_cast<uint32_t>(framebuffer.width) + TPB_1D - 1) / TPB_1D, 
			(static_cast<uint32_t>(framebuffer.height) + TPB_1D - 1) / TPB_1D,  
			1 
		};

		ClearKernel<<<grid, block>>>(color, framebuffer);
		cudaDeviceSynchronize();
	}
}
