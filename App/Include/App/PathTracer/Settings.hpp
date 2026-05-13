#pragma once
#include <cstdint>
#include "Core/Graphics/Cuda/PathTracing/PathTracerDefaults.hpp"

namespace App::PathTracer
{
	struct Settings
	{
		using Defaults = Core::Graphics::Cuda::PathTracerDefaults;

		uint32_t frameWidth = Defaults::FrameWidth;
		uint32_t frameHeight = Defaults::FrameHeight;
		uint32_t samplesPerPixel = Defaults::SamplesPerPixel;
		uint32_t pathDepthLimit = Defaults::PathDepthLimit;
	};
}