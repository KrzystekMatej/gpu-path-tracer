#pragma once
#include <Core/Graphics/Cuda/PathTracing/Memory/ShadowRayQueueView.hpp>
#include <Core/Graphics/Cuda/Runtime/Memory/SharedCounter.hpp>
#include <Core/Graphics/Cuda/Runtime/Memory/DeviceBuffer1D.hpp>

namespace Core::Graphics::Cuda
{
    class ShadowRayQueue
    {
    public:
        ShadowRayQueue() = default;
        ~ShadowRayQueue() = default;

        ShadowRayQueue(const ShadowRayQueue&) = delete;
        ShadowRayQueue& operator=(const ShadowRayQueue&) = delete;

        ShadowRayQueue(ShadowRayQueue&& other) noexcept = default;
        ShadowRayQueue& operator=(ShadowRayQueue&& other) noexcept = default;

        std::expected<void, Core::Utils::Error> Allocate(uint32_t capacity, const Runtime::Stream& stream = Runtime::Stream::Default());
        std::expected<void, Core::Utils::Error> Free(const Runtime::Stream& stream = Runtime::Stream::Default());

        uint32_t GetCapacity() const
        {
            return m_Paths.GetSize();
        }

        ShadowRayQueueView GetView() const
        {
            return ShadowRayQueueView(
                m_Counter.GetDevicePointer(),
                GetCapacity(),
                reinterpret_cast<uint32_t*>(m_Paths.GetData()),
                reinterpret_cast<float*>(m_RadianceXs.GetData()),
                reinterpret_cast<float*>(m_RadianceYs.GetData()),
                reinterpret_cast<float*>(m_RadianceZs.GetData()),
                reinterpret_cast<float*>(m_OriginXs.GetData()),
                reinterpret_cast<float*>(m_OriginYs.GetData()),
                reinterpret_cast<float*>(m_OriginZs.GetData()),
                reinterpret_cast<float*>(m_DirectionXs.GetData()),
                reinterpret_cast<float*>(m_DirectionYs.GetData()),
                reinterpret_cast<float*>(m_DirectionZs.GetData()),
                reinterpret_cast<float*>(m_TMins.GetData()),
                reinterpret_cast<float*>(m_TMaxs.GetData()));
        }

    private:
        Runtime::SharedCounter<uint32_t> m_Counter;

        Runtime::DeviceBuffer1D m_Paths;

        Runtime::DeviceBuffer1D m_RadianceXs;
        Runtime::DeviceBuffer1D m_RadianceYs;
        Runtime::DeviceBuffer1D m_RadianceZs;

        Runtime::DeviceBuffer1D m_OriginXs;
        Runtime::DeviceBuffer1D m_OriginYs;
        Runtime::DeviceBuffer1D m_OriginZs;
        Runtime::DeviceBuffer1D m_DirectionXs;
        Runtime::DeviceBuffer1D m_DirectionYs;
        Runtime::DeviceBuffer1D m_DirectionZs;
        Runtime::DeviceBuffer1D m_TMins;
        Runtime::DeviceBuffer1D m_TMaxs;
    };
}