#include <Core/Graphics/Cuda/PathTracing/Memory/ShadowRayQueue.hpp>
#include <array>

namespace Core::Graphics::Cuda
{
    std::expected<void, Core::Utils::Error> ShadowRayQueue::Allocate(uint32_t capacity, const Runtime::Stream& stream)
    {
        auto allocateResult = [&]() -> std::expected<void, Core::Utils::Error>
        {
            CORE_TRY_DISCARD(m_Counter.Allocate(stream));

            CORE_TRY_DISCARD(m_Paths.Allocate(capacity, sizeof(uint32_t), stream));
            CORE_TRY_DISCARD(m_RadianceXs.Allocate(capacity, sizeof(float), stream));
            CORE_TRY_DISCARD(m_RadianceYs.Allocate(capacity, sizeof(float), stream));
            CORE_TRY_DISCARD(m_RadianceZs.Allocate(capacity, sizeof(float), stream));

            CORE_TRY_DISCARD(m_OriginXs.Allocate(capacity, sizeof(float), stream));
            CORE_TRY_DISCARD(m_OriginYs.Allocate(capacity, sizeof(float), stream));
            CORE_TRY_DISCARD(m_OriginZs.Allocate(capacity, sizeof(float), stream));
            CORE_TRY_DISCARD(m_DirectionXs.Allocate(capacity, sizeof(float), stream));
            CORE_TRY_DISCARD(m_DirectionYs.Allocate(capacity, sizeof(float), stream));
            CORE_TRY_DISCARD(m_DirectionZs.Allocate(capacity, sizeof(float), stream));
            CORE_TRY_DISCARD(m_TMins.Allocate(capacity, sizeof(float), stream));
            CORE_TRY_DISCARD(m_TMaxs.Allocate(capacity, sizeof(float), stream));
            return {};
        }();

        if (!allocateResult)
        {
            (void)Free(stream);
            return std::unexpected(allocateResult.error());
        }

        return {};
    }

    std::expected<void, Core::Utils::Error> ShadowRayQueue::Free(const Runtime::Stream& stream)
    {
        std::array freeResults =
        {
            m_Counter.Free(stream),

            m_Paths.Free(stream),
            m_RadianceXs.Free(stream),
            m_RadianceYs.Free(stream),
            m_RadianceZs.Free(stream),

            m_OriginXs.Free(stream),
            m_OriginYs.Free(stream),
            m_OriginZs.Free(stream),
            m_DirectionXs.Free(stream),
            m_DirectionYs.Free(stream),
            m_DirectionZs.Free(stream),
            m_TMins.Free(stream),
            m_TMaxs.Free(stream),
        };

        for (const auto& result : freeResults)
        {
            if (!result)
                return std::unexpected(result.error());
        }

        return {};
    }
}