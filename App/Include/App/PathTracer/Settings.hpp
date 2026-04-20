#pragma once
#include <cstdint>

namespace App::PathTracer
{
	struct Settings
	{
		uint32_t frameWidth = 1920;
		uint32_t frameHeight = 1080;
		uint32_t samplesPerPixel = 100;
		uint32_t pathDepth = 10;
	};
}