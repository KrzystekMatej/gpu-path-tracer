#pragma once
#include <chrono>
#include <spdlog/spdlog.h>

namespace Core::Utils
{
    class Timer
    {
    public:
        void Start()
        {
            m_Start = std::chrono::high_resolution_clock::now();
        }

        void Stop()
        {
            m_End = std::chrono::high_resolution_clock::now();
            m_Duration = m_End - m_Start;
        }

        float GetElapsedSeconds() const
        {
            return m_Duration.count();
        }

        float GetElapsedMilliseconds() const
        {
            return m_Duration.count() * 1000.0f;
        }
    private:
        std::chrono::time_point<std::chrono::steady_clock> m_Start, m_End;
        std::chrono::duration<float> m_Duration{ 0.0f };
    };
}