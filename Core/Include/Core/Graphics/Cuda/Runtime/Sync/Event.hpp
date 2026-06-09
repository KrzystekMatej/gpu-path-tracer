#pragma once
#include <expected>
#include <Core/Utils/Error.hpp>
#include <cuda_runtime.h>
#include <Core/Graphics/Cuda/Runtime/Sync/Stream.hpp>

namespace Core::Graphics::Cuda::Runtime
{
    class Event
    {
    public:
        static std::expected<Event, Core::Utils::Error> Create();
        ~Event();

        Event(const Event&) = delete;
        Event& operator=(const Event&) = delete;

        Event(Event&& other) noexcept;
        Event& operator=(Event&& other) noexcept;

        std::expected<void, Core::Utils::Error> Record(const Stream& stream = Stream::Default()) const;
        std::expected<void, Core::Utils::Error> Synchronize() const;
        std::expected<bool, Core::Utils::Error> Query() const;

        cudaEvent_t GetRawHandle() const { return m_Event; }
    private:
        friend class Timer;
        Event() = default;
        
        cudaEvent_t m_Event = nullptr;
    };
}