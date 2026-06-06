#pragma once

#include <cstdint>
#include <expected>

#include <Core/Graphics/Cuda/PathTracing/RayQueueView.hpp>
#include <Core/Graphics/Cuda/Runtime/DeviceBuffer1D.hpp>
#include <Core/Graphics/Cuda/Runtime/SharedCounter.hpp>
#include <Core/Graphics/Cuda/Runtime/Stream.hpp>
#include <Core/Utils/Error.hpp>

namespace Core::Graphics::Cuda
{
    class RayQueue
    {
    public:
        RayQueue() = default;
        ~RayQueue() = default;

        RayQueue(const RayQueue&) = delete;
        RayQueue& operator=(const RayQueue&) = delete;

        RayQueue(RayQueue&& other) noexcept = default;
        RayQueue& operator=(RayQueue&& other) noexcept = default;

        std::expected<void, Core::Utils::Error> Allocate(uint32_t capacity, const Runtime::Stream& stream = Runtime::Stream::Default());
        std::expected<void, Core::Utils::Error> Free(const Runtime::Stream& stream = Runtime::Stream::Default());

        uint32_t GetCapacity() const
        {
            return m_Paths.GetSize();
        }

        const Runtime::SharedCounter<uint32_t>& GetCounter() const
        {
            return m_Counter;
        }

        Runtime::SharedCounter<uint32_t>& GetCounter()
        {
            return m_Counter;
        }

        RayQueueView GetView() const
        {
            return RayQueueView(
                m_Counter.GetDevicePointer(),
                GetCapacity(),
                reinterpret_cast<uint32_t*>(m_Paths.GetData()),
                reinterpret_cast<float*>(m_OriginXs.GetData()),
                reinterpret_cast<float*>(m_OriginYs.GetData()),
                reinterpret_cast<float*>(m_OriginZs.GetData()),
                reinterpret_cast<float*>(m_DirectionXs.GetData()),
                reinterpret_cast<float*>(m_DirectionYs.GetData()),
                reinterpret_cast<float*>(m_DirectionZs.GetData()),
                reinterpret_cast<float*>(m_TMins.GetData()),
                reinterpret_cast<float*>(m_TMaxs.GetData()),
                reinterpret_cast<float*>(m_Iors.GetData()));
        }

    private:
        Runtime::SharedCounter<uint32_t> m_Counter;

        Runtime::DeviceBuffer1D m_Paths;
        Runtime::DeviceBuffer1D m_OriginXs;
        Runtime::DeviceBuffer1D m_OriginYs;
        Runtime::DeviceBuffer1D m_OriginZs;
        Runtime::DeviceBuffer1D m_DirectionXs;
        Runtime::DeviceBuffer1D m_DirectionYs;
        Runtime::DeviceBuffer1D m_DirectionZs;
        Runtime::DeviceBuffer1D m_TMins;
        Runtime::DeviceBuffer1D m_TMaxs;
        Runtime::DeviceBuffer1D m_Iors;
    };
}