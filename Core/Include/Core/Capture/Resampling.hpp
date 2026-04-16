#pragma once
#include <vector>
#include <Core/Capture/Types.hpp>

namespace Core::Capture
{
	std::vector<MotionState> ResampleMotion(const std::vector<MotionSample>& samples, float sampleInterval);
}