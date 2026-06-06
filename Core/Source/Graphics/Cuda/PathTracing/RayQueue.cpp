#include <Core/Graphics/Cuda/PathTracing/RayQueue.hpp>

namespace Core::Graphics::Cuda
{
    std::expected<void, Core::Utils::Error> RayQueue::Allocate(uint32_t capacity, const Runtime::Stream& stream)
    {
        CORE_TRY_DISCARD(Free(stream));

        auto allocateResult = [&]() -> std::expected<void, Core::Utils::Error>
        {
            CORE_TRY_DISCARD(m_Counter.Allocate(stream));
            CORE_TRY_DISCARD(m_Paths.Allocate(capacity, sizeof(uint32_t), stream));
            CORE_TRY_DISCARD(m_OriginXs.Allocate(capacity, sizeof(float), stream));
            CORE_TRY_DISCARD(m_OriginYs.Allocate(capacity, sizeof(float), stream));
            CORE_TRY_DISCARD(m_OriginZs.Allocate(capacity, sizeof(float), stream));
            CORE_TRY_DISCARD(m_DirectionXs.Allocate(capacity, sizeof(float), stream));
            CORE_TRY_DISCARD(m_DirectionYs.Allocate(capacity, sizeof(float), stream));
            CORE_TRY_DISCARD(m_DirectionZs.Allocate(capacity, sizeof(float), stream));
            CORE_TRY_DISCARD(m_TMins.Allocate(capacity, sizeof(float), stream));
            CORE_TRY_DISCARD(m_TMaxs.Allocate(capacity, sizeof(float), stream));
            CORE_TRY_DISCARD(m_Iors.Allocate(capacity, sizeof(float), stream));
            return {};
        }();

        if (!allocateResult)
        {
            (void)Free(stream);
            return std::unexpected(allocateResult.error());
        }

        return {};
    }

    std::expected<void, Core::Utils::Error> RayQueue::Free(const Runtime::Stream& stream)
    {
        auto counterResult = m_Counter.Free(stream);
        auto pathResult = m_Paths.Free(stream);
        auto originXResult = m_OriginXs.Free(stream);
        auto originYResult = m_OriginYs.Free(stream);
        auto originZResult = m_OriginZs.Free(stream);
        auto directionXResult = m_DirectionXs.Free(stream);
        auto directionYResult = m_DirectionYs.Free(stream);
        auto directionZResult = m_DirectionZs.Free(stream);
        auto tMinResult = m_TMins.Free(stream);
        auto tMaxResult = m_TMaxs.Free(stream);
        auto iorResult = m_Iors.Free(stream);

        if (!counterResult)
            return std::unexpected(counterResult.error());

        if (!pathResult)
            return std::unexpected(pathResult.error());

        if (!originXResult)
            return std::unexpected(originXResult.error());

        if (!originYResult)
            return std::unexpected(originYResult.error());

        if (!originZResult)
            return std::unexpected(originZResult.error());

        if (!directionXResult)
            return std::unexpected(directionXResult.error());

        if (!directionYResult)
            return std::unexpected(directionYResult.error());

        if (!directionZResult)
            return std::unexpected(directionZResult.error());

        if (!tMinResult)
            return std::unexpected(tMinResult.error());

        if (!tMaxResult)
            return std::unexpected(tMaxResult.error());

        if (!iorResult)
            return std::unexpected(iorResult.error());

        return {};
    }
}