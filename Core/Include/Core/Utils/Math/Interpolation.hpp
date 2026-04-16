#pragma once
#include <Core/External/Glm.hpp>

namespace Core::Utils::Math::Interpolation
{
	template<typename T>
	inline T Lerp(const T& a, const T& b, float t)
	{
		return glm::mix(a, b, t);
	}

	template<typename T>
	inline T LerpClamped(const T& a, const T& b, float t)
	{
		t = glm::clamp(t, 0.0f, 1.0f);
		return glm::mix(a, b, t);
	}

	inline glm::quat EnsureSameHemisphere(glm::quat a, glm::quat b)
	{
		return (glm::dot(a, b) < 0.0f) ? -b : b;
	}

	
	inline glm::quat SlerpShortest(glm::quat a, glm::quat b, float t)
	{
		b = EnsureSameHemisphere(a, b);
		return glm::normalize(glm::slerp(a, b, t));
	}

	inline glm::quat NlerpShortest(glm::quat a, glm::quat b, float t)
	{
		b = EnsureSameHemisphere(a, b);
		return glm::normalize(glm::mix(a, b, t));
	}
}