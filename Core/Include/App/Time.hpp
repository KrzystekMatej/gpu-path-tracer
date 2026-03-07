#pragma once

namespace Core::App
{
	class Time
	{
	public:
		void Update();
		float GetDeltaTime() const { return m_DeltaTime; }
		float GetElapsedTime() const;
	private:
		double m_LastFrameTime = 0.0;
		float m_DeltaTime = 0.0f;
	};
}
