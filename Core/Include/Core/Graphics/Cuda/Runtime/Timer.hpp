#pragma once
#include <Core/Graphics/Cuda/Runtime/Event.hpp>

namespace Core::Graphics::Cuda::Runtime
{
    class Timer
    {
    public:
        static std::expected<Timer, Core::Utils::Error> Create();

        std::expected<void, Core::Utils::Error> Start(const Stream& stream = Stream::Default());
        std::expected<void, Core::Utils::Error> Stop(const Stream& stream = Stream::Default());
        float GetElapsedMilliseconds() const { return m_ElapsedTime; }
        float GetElapsedSeconds() const { return m_ElapsedTime * 0.001f; }
    private:
        friend class Profiler;
        Timer() = default;    

        Event m_StartEvent;
        Event m_StopEvent;
        float m_ElapsedTime = 0.0f;
    };
}