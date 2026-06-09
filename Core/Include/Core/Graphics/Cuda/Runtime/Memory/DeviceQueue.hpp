#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <type_traits>

#include <Core/Graphics/Cuda/Runtime/Memory/DeviceBuffer1D.hpp>
#include <Core/Graphics/Cuda/Runtime/Memory/DeviceQueueView.hpp>
#include <Core/Graphics/Cuda/Runtime/Memory/SharedCounter.hpp>
#include <Core/Graphics/Cuda/Runtime/Sync/Stream.hpp>
#include <Core/Utils/Error.hpp>

namespace Core::Graphics::Cuda::Runtime
{
    class DeviceQueue
    {
    public:
        DeviceQueue() = default;
        ~DeviceQueue() = default;

        DeviceQueue(const DeviceQueue&) = delete;
        DeviceQueue& operator=(const DeviceQueue&) = delete;

        DeviceQueue(DeviceQueue&& other) noexcept = default;
        DeviceQueue& operator=(DeviceQueue&& other) noexcept = default;

        std::expected<void, Core::Utils::Error> Allocate(uint32_t capacity, uint32_t elementSize, const Stream& stream = Stream::Default());
        std::expected<void, Core::Utils::Error> Free(const Stream& stream = Stream::Default());

        const DeviceBuffer1D& GetBuffer() const
        {
            return m_Buffer;
        }

        DeviceBuffer1D& GetBuffer()
        {
            return m_Buffer;
        }

        const SharedCounter<uint32_t>& GetCounter() const
        {
            return m_Counter;
        }

        SharedCounter<uint32_t>& GetCounter()
        {
            return m_Counter;
        }

        uint32_t GetCapacity() const
        {
            return m_Buffer.GetSize();
        }

        uint32_t GetElementSize() const
        {
            return m_Buffer.GetElementSize();
        }

        template<typename T>
        DeviceQueueView<T> GetView() const
        {
            static_assert(!std::is_void_v<T>);
            assert(m_Buffer.GetElementSize() == sizeof(T));

            return DeviceQueueView<T>(
                reinterpret_cast<T*>(m_Buffer.GetData()),
                GetCapacity(),
                m_Counter.GetView());
        }

    private:
        DeviceBuffer1D m_Buffer;
        SharedCounter<uint32_t> m_Counter;
    };
}