#include <Core/Graphics/Cuda/PathTracing/Memory/RegenQueue.hpp>


namespace Core::Graphics::Cuda
{
    std::expected<void, Core::Utils::Error> RegenQueue::Allocate(uint32_t capacity, const Runtime::Stream& stream)
    {
        auto counterResult = m_Counter.Allocate(stream);
        if (!counterResult)
        {
            return std::unexpected(counterResult.error());
        }

        auto pathsResult = m_Paths.Allocate(capacity, sizeof(uint32_t), stream);
        if (!pathsResult)
        {
            (void)m_Counter.Free(stream);
            return std::unexpected(pathsResult.error());
        }

        return {};
    }

    std::expected<void, Core::Utils::Error> RegenQueue::Free(const Runtime::Stream& stream)
    {
        auto pathsResult = m_Paths.Free(stream);
        auto counterResult = m_Counter.Free(stream);
        
        if (!pathsResult)
            return std::unexpected(pathsResult.error());

        if (!counterResult)
            return std::unexpected(counterResult.error());

        return {};
    }
}