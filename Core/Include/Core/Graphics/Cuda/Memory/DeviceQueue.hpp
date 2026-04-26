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

        std::expected<void, Core::Utils::Error> ResetCounter();
        std::expected<void, Core::Utils::Error> SyncCounterFromDevice();
        std::expected<void, Core::Utils::Error> SyncCounterFromHost();

        const DeviceBuffer1D& GetBuffer() const { return m_Buffer; }
        DeviceBuffer1D& GetBuffer() { return m_Buffer; }

        const SharedCounter& GetCounter() const { return m_Counter; }
        SharedCounter& GetCounter() { return m_Counter; }

		size_t GetCapacity() const { return m_Buffer.GetSize(); }
        size_t GetElementSize() const { return m_Buffer.GetElementSize(); }

        template<typename T>
        DeviceQueueView<T> GetView() const
        {
            static_assert(!std::is_void_v<T>);
            assert(m_Buffer.GetElementSize() == sizeof(T));

            return
            {
                reinterpret_cast<T*>(m_Buffer.GetData()),
				GetCapacity(),
                m_Counter.GetView()
            };
        }

    private:
        DeviceBuffer1D m_Buffer;
        SharedCounter m_Counter;
    };
}