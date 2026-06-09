#include <Core/Graphics/Cuda/Runtime/Sync/Event.hpp>
#include <utility>

namespace Core::Graphics::Cuda::Runtime
{
    std::expected<Event, Core::Utils::Error> Event::Create()
    {
        Event event;
        CUDA_TRY("cudaEventCreate", cudaEventCreate(&event.m_Event));
        return event;
    }

    Event::~Event()
    {
        if (m_Event)
        {
            cudaEventDestroy(m_Event);
            m_Event = nullptr;
        }
    }

    Event::Event(Event&& other) noexcept
        : m_Event(std::exchange(other.m_Event, nullptr))
    {
    }

    Event& Event::operator=(Event&& other) noexcept
    {
        if (this != &other)
        {
            if (m_Event)
                cudaEventDestroy(m_Event);
            
            m_Event = std::exchange(other.m_Event, nullptr);
        }
        return *this;
    }

    std::expected<void, Core::Utils::Error> Event::Record(const Stream& stream) const
    {
        CUDA_TRY("cudaEventRecord", cudaEventRecord(m_Event, stream.GetRawHandle()));
        return {};
    }

    std::expected<void, Core::Utils::Error> Event::Synchronize() const
    {
        CUDA_TRY("cudaEventSynchronize", cudaEventSynchronize(m_Event));
        return {};
    }

    std::expected<bool, Core::Utils::Error> Event::Query() const
    {
        cudaError_t result = cudaEventQuery(m_Event);
        if (result == cudaSuccess)
            return true;
        else if (result == cudaErrorNotReady)
            return false;
        else
            return std::unexpected(Utils::MakeCudaError("cudaEventQuery", result));
    }
}