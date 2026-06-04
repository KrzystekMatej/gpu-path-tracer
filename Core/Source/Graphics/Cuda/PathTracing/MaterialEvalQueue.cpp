#include <Core/Graphics/Cuda/PathTracing/MaterialEvalQueue.hpp>

namespace Core::Graphics::Cuda
{
    namespace
    {
        std::expected<void, Core::Utils::Error> AllocateField(
            Runtime::DeviceBuffer1D& buffer,
            uint32_t capacity,
            uint32_t elementSize,
            const Runtime::Stream& stream)
        {
            return buffer.Allocate(capacity, elementSize, stream);
        }
    }

    MaterialEvalQueue::~MaterialEvalQueue()
    {
        (void)Free();
    }

    std::expected<void, Core::Utils::Error> MaterialEvalQueue::Allocate(uint32_t capacity, const Runtime::Stream& stream)
    {
        CORE_TRY_DISCARD(Free(stream));

        auto counterResult = m_Counter.Allocate();
        if (!counterResult)
            return std::unexpected(counterResult.error());

        auto pathResult = AllocateField(m_Paths, capacity, sizeof(uint32_t), stream);
        if (!pathResult)
        {
            (void)Free(stream);
            return std::unexpected(pathResult.error());
        }

        auto triangleResult = AllocateField(m_Triangles, capacity, sizeof(uint32_t), stream);
        if (!triangleResult)
        {
            (void)Free(stream);
            return std::unexpected(triangleResult.error());
        }

        auto materialResult = AllocateField(m_Materials, capacity, sizeof(uint32_t), stream);
        if (!materialResult)
        {
            (void)Free(stream);
            return std::unexpected(materialResult.error());
        }

        auto uResult = AllocateField(m_Us, capacity, sizeof(float), stream);
        if (!uResult)
        {
            (void)Free(stream);
            return std::unexpected(uResult.error());
        }

        auto vResult = AllocateField(m_Vs, capacity, sizeof(float), stream);
        if (!vResult)
        {
            (void)Free(stream);
            return std::unexpected(vResult.error());
        }

        return {};
    }

    std::expected<void, Core::Utils::Error> MaterialEvalQueue::Free(const Runtime::Stream& stream)
    {
        auto counterResult = m_Counter.Free();
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