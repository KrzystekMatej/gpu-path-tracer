#pragma once
#include <Core/External/Glm.hpp>

namespace Core::Capture
{
	struct MotionState
	{
		glm::vec3 position = glm::vec3(0.0f);
		glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	};

	struct MotionSample
	{
		MotionState state;
		float time;
	};
}