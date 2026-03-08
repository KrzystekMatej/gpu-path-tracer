#pragma once
#include "External/Glm.hpp"

namespace Core::Utils::Math
{
	struct CoordinateSystem
	{
		static constexpr glm::vec3 Right{ 1.0f, 0.0f, 0.0f };
		static constexpr glm::vec3 Up{ 0.0f, 1.0f, 0.0f };
		static constexpr glm::vec3 Forward{ 0.0f, 0.0f, -1.0f };
	};
}