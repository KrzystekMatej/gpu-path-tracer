#pragma once
#include "External/Glm.hpp"

namespace Core::Graphics
{
	struct Light
	{
		glm::vec3 position;
		glm::vec3 color;
		float intensity;
	};
}