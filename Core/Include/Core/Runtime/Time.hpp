#pragma once
#include <chrono>

namespace Core::Runtime
{
	class Time
	{
	public:
		Time();

		void Update();
		float GetDeltaTime() const { return m_DeltaTime; }
		float GetDeltaTimeMs() const { return m_DeltaTime * 1000.0f; }
		float GetElapsedTime() const { return m_ElapsedTime; }
		float GetFps() const { return m_Fps; }
	private:
		using Clock = std::chrono::steady_clock;
		using TimePoint = Clock::time_point;

		TimePoint m_StartTime;
		TimePoint m_LastFrameTime;

		float m_DeltaTime = 0.0f;
		float m_ElapsedTime = 0.0f;

		float m_SmoothedDeltaTime = 1.0f / 60.0f;
		float m_Fps = 60.0f;
	};
}
