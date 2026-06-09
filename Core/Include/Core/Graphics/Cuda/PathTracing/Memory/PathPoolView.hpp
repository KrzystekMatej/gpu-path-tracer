#pragma once
#include <Core/Graphics/Cuda/PathTracing/Memory/PathData.hpp>
#include <Core/Graphics/Cuda/Runtime/Memory/DeviceBuffer1DView.hpp>

namespace Core::Graphics::Cuda
{
	struct PathPoolView
	{
		Runtime::DeviceBuffer1DView<Pixel> pixels;
		Runtime::DeviceBuffer1DView<Random> randoms;
	};
}
