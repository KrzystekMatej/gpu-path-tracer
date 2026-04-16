#pragma once
#include <cstdint>
#include <expected>
#include <Core/Graphics/Cuda/Memory/CounterView.hpp>
#include <Core/Graphics/Cuda/Memory/DeviceBuffer1D.hpp>
#include <Core/Utils/Error.hpp>

namespace Core::Graphics::Cuda::Memory
{
    class SharedCounter
    {
    public:
        SharedCounter() = default;
        ~SharedCounter();

        SharedCounter(const SharedCounter&) = delete;
        SharedCounter& operator=(const SharedCounter&) = delete;

        SharedCounter(SharedCounter&& other) noexcept;
        SharedCounter& operator=(SharedCounter&& other) noexcept;

        std::expected<void, Core::Utils::Error> Allocate();
        std::expected<void, Core::Utils::Error> Free();

        std::expected<void, Core::Utils::Error> Reset();
        std::expected<void, Core::Utils::Error> SyncFromDevice();
        std::expected<void, Core::Utils::Error> SyncFromHost();

        uint32_t* GetDevicePointer() const;
        const DeviceBuffer1D& GetDeviceBuffer() const { return m_DeviceBuffer; }

        uint32_t GetHostValue() const { return m_HostValue; }
        void SetHostValue(uint32_t value) { m_HostValue = value; }

        CounterView GetView() const
        {
            return { reinterpret_cast<uint32_t*>(m_DeviceBuffer.GetData()) };
        }
    private:
        DeviceBuffer1D m_DeviceBuffer;
        uint32_t m_HostValue = 0;
    };
}