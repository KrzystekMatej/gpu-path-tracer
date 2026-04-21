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

	struct CameraShotSettings
	{
		float fovY = glm::radians(45.0f);
		float nearPlane = 0.1f;
		float farPlane = 100.0f;
		uint32_t width = 1920;
		uint32_t height = 1080;
	};
}