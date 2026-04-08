#include <Core/Runtime/Time.hpp>
#include <GLFW/glfw3.h>

namespace Core::Runtime
{
	void Time::Update()
	{
		double currentTime = glfwGetTime();
		m_DeltaTime = static_cast<float>(currentTime - m_LastFrameTime);
		m_LastFrameTime = currentTime;
	}

	float Time::GetElapsedTime() const { return static_cast<float>(glfwGetTime()); }
}
