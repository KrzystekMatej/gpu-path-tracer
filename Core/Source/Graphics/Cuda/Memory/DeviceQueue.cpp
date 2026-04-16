#include <Core/Graphics/Cuda/Memory/DeviceQueue.hpp>
#include <utility>

namespace Core::Graphics::Cuda::Memory
{
    DeviceQueue::~DeviceQueue()
    {
        Free();
    }

    DeviceQueue::DeviceQueue(DeviceQueue&& other) noexcept
        : m_Buffer(std::move(other.m_Buffer))
        , m_Counter(std::move(other.m_Counter))
    {
    }

    DeviceQueue& DeviceQueue::operator=(DeviceQueue&& other) noexcept
    {
        if (this != &other)
        {
            Free();
            m_Buffer = std::move(other.m_Buffer);
            m_Counter = std::move(other.m_Counter);
        }

        return *this;
    }

    std::expected<void, Core::Utils::Error> DeviceQueue::Allocate(size_t capacity, size_t elementSize)
    {
        auto freeResult = Free();
        if (!freeResult)
            return std::unexpected(freeResult.error());

        auto bufferAllocateResult = m_Buffer.Allocate(capacity * elementSize, elementSize);
        if (!bufferAllocateResult)
            return std::unexpected(bufferAllocateResult.error());

        auto counterAllocateResult = m_Counter.Allocate();
        if (!counterAllocateResult)
        {
            m_Buffer.Free();
            return std::unexpected(counterAllocateResult.error());
        }

        return {};
    }

    std::expected<void, Core::Utils::Error> DeviceQueue::Free()
    {
        auto bufferFreeResult = m_Buffer.Free();
        auto counterFreeResult = m_Counter.Free();

        if (!bufferFreeResult)
            return std::unexpected(bufferFreeResult.error());

        if (!counterFreeResult)
            return std::unexpected(counterFreeResult.error());

        return {};
    }

    std::expected<void, Core::Utils::Error> DeviceQueue::ResetCounter()
    {
        return m_Counter.Reset();
    }

    std::expected<void, Core::Utils::Error> DeviceQueue::SyncCounterFromDevice()
    {
        return m_Counter.SyncFromDevice();
    }

    std::expected<void, Core::Utils::Error> DeviceQueue::SyncCounterFromHost()
    {
        return m_Counter.SyncFromHost();
    }

    size_t DeviceQueue::GetCapacity() const
    {
        const size_t elementSize = m_Buffer.GetElementSize();
        assert(elementSize != 0);

        return m_Buffer.GetSize() / elementSize;
    }
}