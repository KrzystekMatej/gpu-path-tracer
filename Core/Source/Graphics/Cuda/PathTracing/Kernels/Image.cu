#include <Core/Graphics/Cuda/PathTracing/Kernels/LaunchUtils.cuh>
#include <Core/Graphics/Cuda/PathTracing/Kernels.hpp>

namespace Core::Graphics::Cuda::Kernels
{
	__global__ void ClearKernel(uchar4 color, Runtime::DeviceBuffer1DView<uchar4> framebuffer)
	{
		uint32_t pixelIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (pixelIndex >= framebuffer.GetSize()) return;

		framebuffer.At(pixelIndex) = color;
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
		Runtime::DeviceBuffer1DView<float4> accumulationBuffer,
		float invSpp,
		Runtime::DeviceBuffer1DView<uchar4> framebuffer)
	{
		uint32_t pixelIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (pixelIndex >= framebuffer.GetSize()) return;

		const float4 hdrColor = accumulationBuffer.At(pixelIndex) * invSpp;
		const float4 ldrColor = LinearToSrgb(TonemapReinhard(hdrColor), 2.2f);
		framebuffer.At(pixelIndex) = make_uchar4(
			static_cast<unsigned char>(clamp(ldrColor.x * 255.0f, 0.0f, 255.0f)),
			static_cast<unsigned char>(clamp(ldrColor.y * 255.0f, 0.0f, 255.0f)),
			static_cast<unsigned char>(clamp(ldrColor.z * 255.0f, 0.0f, 255.0f)),
			255);
	}
    
	void Clear(cudaStream_t stream, uchar4 color, Runtime::DeviceBuffer1DView<uchar4> framebuffer)
	{

		dim3 block(LaunchUtils::ClearThreadsPerBlock);
		dim3 grid(LaunchUtils::GetBlockCount(framebuffer.GetSize(), block.x));
		ClearKernel<<<grid, block, 0, stream>>>(color, framebuffer);
	}
    
	void PostprocessAccumulatedRadiance(
		cudaStream_t stream,
		Runtime::DeviceBuffer1DView<float4> accumulationBuffer,
		float invSpp,
		Runtime::DeviceBuffer1DView<uchar4> framebuffer)
	{
		dim3 block(LaunchUtils::PostprocessThreadsPerBlock);
		dim3 grid(LaunchUtils::GetBlockCount(accumulationBuffer.GetSize(), block.x));

		PostprocessKernel<<<grid, block, 0, stream>>>(accumulationBuffer, invSpp, framebuffer);
	}
}
