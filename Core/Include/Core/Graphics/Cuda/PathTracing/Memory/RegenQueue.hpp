#pragma once

#include <cstdint>
#include <expected>

#include <Core/Graphics/Cuda/PathTracing/Memory/RegenQueueView.hpp>
#include <Core/Graphics/Cuda/Runtime/Memory/DeviceBuffer1D.hpp>
#include <Core/Graphics/Cuda/Runtime/Memory/SharedCounter.hpp>
#include <Core/Graphics/Cuda/Runtime/Sync/Stream.hpp>
#include <Core/Utils/Error.hpp>

namespace Core::Graphics::Cuda
{
    class RegenQueue
    {
    public:
        RegenQueue() = default;
        ~RegenQueue() = default;

        RegenQueue(const RegenQueue&) = delete;
        RegenQueue& operator=(const RegenQueue&) = delete;

        RegenQueue(RegenQueue&& other) noexcept = default;
        RegenQueue& operator=(RegenQueue&& other) noexcept = default;

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

        RegenQueueView GetView() const
        {
            return RegenQueueView(
                m_Counter.GetDevicePointer(),
                GetCapacity(),
                reinterpret_cast<uint32_t*>(m_Paths.GetData()));
        }

    private:
        Runtime::SharedCounter<uint32_t> m_Counter;
        Runtime::DeviceBuffer1D m_Paths;
    };
}