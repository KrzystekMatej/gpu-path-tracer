#include <Core/Graphics/Cuda/Runtime/DeviceQueue.hpp>
#include <utility>

namespace Core::Graphics::Cuda::Runtime
{
    std::expected<void, Core::Utils::Error> DeviceQueue::Allocate(uint32_t capacity, uint32_t elementSize, const Stream& stream)
    {
        CORE_TRY_DISCARD(Free(stream));
        CORE_TRY_DISCARD(m_Buffer.Allocate(capacity, elementSize, stream));

        auto counterAllocateResult = m_Counter.Allocate(stream);
        if (!counterAllocateResult)
        {
            (void)m_Buffer.Free(stream);
            return std::unexpected(counterAllocateResult.error());
        }

        return {};
    }

    std::expected<void, Core::Utils::Error> DeviceQueue::Free(const Stream& stream)
    {
        auto bufferFreeResult = m_Buffer.Free(stream);
        auto counterFreeResult = m_Counter.Free(stream);

        if (!bufferFreeResult)
            return std::unexpected(bufferFreeResult.error());

        if (!counterFreeResult)
            return std::unexpected(counterFreeResult.error());

        return {};
    }
}