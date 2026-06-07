#pragma once
#include <Core/Graphics/Cuda/PathTracing/PathData.hpp>
#include <Core/Graphics/Cuda/Runtime/DeviceBuffer1DView.hpp>

namespace Core::Graphics::Cuda
{
	struct PathPoolView
	{
		Runtime::DeviceBuffer1DView<Pixel> pixels;
		Runtime::DeviceBuffer1DView<Contribution> contributions;
		Runtime::DeviceBuffer1DView<Random> randoms;
	};
}
