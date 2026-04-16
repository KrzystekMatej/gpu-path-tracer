#include <Core/Runtime/Time.hpp>

namespace Core::Runtime
{
	Time::Time()
		: m_StartTime(Clock::now()),
		  m_LastFrameTime(m_StartTime)
	{
	}

	void Time::Update()
	{
		const TimePoint currentTime = Clock::now();

		m_DeltaTime = std::chrono::duration<float>(currentTime - m_LastFrameTime).count();
		m_ElapsedTime = std::chrono::duration<float>(currentTime - m_StartTime).count();

		m_LastFrameTime = currentTime;

		if (m_DeltaTime <= 0.0f)
			return;

		constexpr float alpha = 0.1f;
		m_SmoothedDeltaTime = alpha * m_DeltaTime + (1.0f - alpha) * m_SmoothedDeltaTime;
		m_Fps = 1.0f / m_SmoothedDeltaTime;
	}
}
