#pragma once
#include <cstdint>
#include <filesystem>
#include <string_view>
#include "Core/Graphics/Cuda/PathTracing/PathTracerDefaults.hpp"

namespace App::PathTracer
{
	struct Settings
	{
		using Defaults = Core::Graphics::Cuda::PathTracerDefaults;
		static constexpr std::string_view PathTracerOutputFolder = "Output";

		uint32_t frameWidth = Defaults::FrameWidth;
		uint32_t frameHeight = Defaults::FrameHeight;
		uint32_t samplesPerPixel = Defaults::SamplesPerPixel;
		uint32_t pathDepthLimit = Defaults::PathDepthLimit;
		std::string renderBatchName = "Render";
	};
}