#pragma once
#include <Core/External/Glm.hpp>

namespace Core::Graphics::Common
{
	struct Light
	{
		glm::vec3 position;
		glm::vec3 color;
		float intensity;
	};
}