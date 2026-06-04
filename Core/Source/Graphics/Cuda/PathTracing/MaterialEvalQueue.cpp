#include <Core/Graphics/Cuda/PathTracing/MaterialEvalQueue.hpp>

namespace Core::Graphics::Cuda
{
    std::expected<void, Core::Utils::Error> MaterialEvalQueue::Allocate(uint32_t capacity, const Runtime::Stream& stream)
    {
        CORE_TRY_DISCARD(Free(stream));

        auto counterResult = m_Counter.Allocate(stream);
        if (!counterResult)
            return std::unexpected(counterResult.error());

        auto pathResult = m_Paths.Allocate(capacity, sizeof(uint32_t), stream);
        if (!pathResult)
        {
            (void)Free(stream);
            return std::unexpected(pathResult.error());
        }

        auto triangleResult = m_Triangles.Allocate(capacity, sizeof(uint32_t), stream);
        if (!triangleResult)
        {
            (void)Free(stream);
            return std::unexpected(triangleResult.error());
        }

        auto materialResult = m_Materials.Allocate(capacity, sizeof(uint32_t), stream);
        if (!materialResult)
        {
            (void)Free(stream);
            return std::unexpected(materialResult.error());
        }

        auto uResult = m_Us.Allocate(capacity, sizeof(float), stream);
        if (!uResult)
        {
            (void)Free(stream);
            return std::unexpected(uResult.error());
        }

        auto vResult = m_Vs.Allocate(capacity, sizeof(float), stream);
        if (!vResult)
        {
            (void)Free(stream);
            return std::unexpected(vResult.error());
        }

        return {};
    }

    std::expected<void, Core::Utils::Error> MaterialEvalQueue::Free(const Runtime::Stream& stream)
    {
        auto counterResult = m_Counter.Free(stream);
        auto pathResult = m_Paths.Free(stream);
        auto triangleResult = m_Triangles.Free(stream);
        auto materialResult = m_Materials.Free(stream);
        auto uResult = m_Us.Free(stream);
        auto vResult = m_Vs.Free(stream);

        if (!counterResult)
            return std::unexpected(counterResult.error());

        if (!pathResult)
            return std::unexpected(pathResult.error());

        if (!triangleResult)
            return std::unexpected(triangleResult.error());

        if (!materialResult)
            return std::unexpected(materialResult.error());

        if (!uResult)
            return std::unexpected(uResult.error());

        if (!vResult)
            return std::unexpected(vResult.error());

        return {};
    }
}