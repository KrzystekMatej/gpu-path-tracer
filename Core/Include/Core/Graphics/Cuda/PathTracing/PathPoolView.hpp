#pragma once
#include <Core/Graphics/Cuda/PathTracing/PathData.hpp>
#include <Core/Graphics/Cuda/Memory/DeviceBuffer1DView.hpp>

namespace Core::Graphics::Cuda
{
	struct PathPoolView
	{
		Memory::DeviceBuffer1DView<Sample> samples;
		Memory::DeviceBuffer1DView<Ray> rays;
		Memory::DeviceBuffer1DView<Contribution> contributions;
		Memory::DeviceBuffer1DView<Random> randoms;
		Memory::DeviceBuffer1DView<PathFlags> pathFlags;
	};
}