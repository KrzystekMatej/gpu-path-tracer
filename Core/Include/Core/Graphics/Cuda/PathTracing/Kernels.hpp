#pragma once
#include <cuda_runtime.h>
#include <Core/Graphics/Cuda/Memory/DeviceBuffer2DView.hpp>

namespace Core::Graphics::Cuda::Kernels
{
	void Clear(uchar4 color, Memory::DeviceBuffer2DView<uchar4> framebuffer);
}