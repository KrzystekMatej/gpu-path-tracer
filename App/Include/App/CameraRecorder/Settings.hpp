#pragma once
#include <cstdint>

namespace App::CameraRecorder
{
	struct Settings
	{
		static constexpr uint32_t MinTargetFps = 1;
		static constexpr uint32_t MaxTargetFps = 120; 

		uint32_t targetFps = 24;
	};
}