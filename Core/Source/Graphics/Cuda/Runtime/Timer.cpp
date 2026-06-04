#include <Core/Graphics/Cuda/Runtime/Timer.hpp>

namespace Core::Graphics::Cuda::Runtime
{
    std::expected<Timer, Core::Utils::Error> Timer::Create()
    {
        Timer timer;
        CORE_TRY(startEvent, Event::Create());
        CORE_TRY(stopEvent, Event::Create());
        timer.m_StartEvent = std::move(startEvent);
        timer.m_StopEvent = std::move(stopEvent);
        return timer;
    }

    std::expected<void, Core::Utils::Error> Timer::Start(const Stream& stream)
    {
        m_ElapsedTime = 0.0f;
        return m_StartEvent.Record(stream);
    }

    std::expected<void, Core::Utils::Error> Timer::Stop(const Stream& stream)
    {
        CORE_TRY_DISCARD(m_StopEvent.Record(stream));
        CORE_TRY_DISCARD(m_StopEvent.Synchronize());

        float milliseconds = 0.0f;
        CUDA_TRY("cudaEventElapsedTime", cudaEventElapsedTime(
            &milliseconds,
            m_StartEvent.GetRawHandle(),
            m_StopEvent.GetRawHandle()
        ));

        m_ElapsedTime = milliseconds;
        return {};
    }
}