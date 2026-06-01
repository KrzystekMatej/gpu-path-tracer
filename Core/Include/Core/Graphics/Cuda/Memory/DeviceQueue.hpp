#pragma once
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <expected>
#include <type_traits>
#include <Core/Graphics/Cuda/Memory/DeviceQueueView.hpp>
#include <Core/Graphics/Cuda/Memory/DeviceBuffer1D.hpp>
#include <Core/Graphics/Cuda/Memory/SharedCounter.hpp>
#include <Core/Utils/Error.hpp>

namespace Core::Graphics::Cuda::Memory
{
    class DeviceQueue
    {
    public:
        DeviceQueue() = default;
        ~DeviceQueue();

        DeviceQueue(const DeviceQueue&) = delete;
        DeviceQueue& operator=(const DeviceQueue&) = delete;

        DeviceQueue(DeviceQueue&& other) noexcept;
        DeviceQueue& operator=(DeviceQueue&& other) noexcept;

        std::expected<void, Core::Utils::Error> Allocate(size_t capacity, size_t elementSize);
        std::expected<void, Core::Utils::Error> Free();

        std::expected<void, Core::Utils::Error> ResetCounter() { return m_Counter.Reset(); }
        std::expected<void, Core::Utils::Error> SyncCounterFromDevice() { return m_Counter.SyncFromDevice(); }
        std::expected<void, Core::Utils::Error> SyncCounterFromHost() { return m_Counter.SyncFromHost(); }

        const DeviceBuffer1D& GetBuffer() const { return m_Buffer; }
        DeviceBuffer1D& GetBuffer() { return m_Buffer; }

        const SharedCounter<uint32_t>& GetCounter() const { return m_Counter; }
        SharedCounter<uint32_t>& GetCounter() { return m_Counter; }
        uint32_t GetCounterHostValue() const { return m_Counter.GetHostValue(); }
        void SetCounterHostValue(uint32_t value) { m_Counter.SetHostValue(value); }

		size_t GetCapacity() const { return m_Buffer.GetSize(); }
        size_t GetElementSize() const { return m_Buffer.GetElementSize(); }

        template<typename T>
        DeviceQueueView<T> GetView() const
        {
            static_assert(!std::is_void_v<T>);
            assert(m_Buffer.GetElementSize() == sizeof(T));

            return
            DeviceQueueView<T>(reinterpret_cast<T*>(m_Buffer.GetData()), GetCapacity(), m_Counter.GetView());
        }

    private:
        DeviceBuffer1D m_Buffer;
        SharedCounter<uint32_t> m_Counter;
    };
}