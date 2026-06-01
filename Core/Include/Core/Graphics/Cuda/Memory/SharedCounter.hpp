#pragma once
#include <cstdint>
#include <expected>
#include <utility>
#include <cassert>

#include <Core/Graphics/Cuda/Memory/CounterView.hpp>
#include <Core/Graphics/Cuda/Memory/DeviceBuffer1D.hpp>
#include <Core/Graphics/Cuda/Utils/Error.hpp>
#include <Core/Utils/Error.hpp>

namespace Core::Graphics::Cuda::Memory
{
    template<typename T>
    class SharedCounter
    {
    public:
        SharedCounter() = default;
        ~SharedCounter()
        {
            Free();
        }

        SharedCounter(const SharedCounter&) = delete;
        SharedCounter& operator=(const SharedCounter&) = delete;

        SharedCounter(SharedCounter&& other) noexcept
            : m_DeviceBuffer(std::move(other.m_DeviceBuffer))
            , m_HostValue(std::exchange(other.m_HostValue, 0))
        {
        }

        SharedCounter& operator=(SharedCounter&& other) noexcept
        {
            if (this != &other)
            {
                Free();
                m_DeviceBuffer = std::move(other.m_DeviceBuffer);
                m_HostValue = std::exchange(other.m_HostValue, 0);
            }

            return *this;
        }

        std::expected<void, Core::Utils::Error> Allocate()
        {
            CORE_TRY_DISCARD(Free());
            m_HostValue = 0;
            CORE_TRY_DISCARD(m_DeviceBuffer.Allocate(1, sizeof(T)));
            return Reset();
        }

        std::expected<void, Core::Utils::Error> Free()
        {
            m_HostValue = 0;
            return m_DeviceBuffer.Free();
        }

        std::expected<void, Core::Utils::Error> Reset()
        {
            m_HostValue = 0;
            return m_DeviceBuffer.MemsetBytesSync(0);
        }

        std::expected<void, Core::Utils::Error> SyncFromDevice()
        {
            assert(m_DeviceBuffer.GetData() != nullptr);


            CUDA_TRY("cudaMemcpy", cudaMemcpy(
                &m_HostValue,
                m_DeviceBuffer.GetData(),
                m_DeviceBuffer.GetSize() * m_DeviceBuffer.GetElementSize(),
                cudaMemcpyKind::cudaMemcpyDeviceToHost));


            return {};
        }

        std::expected<void, Core::Utils::Error> SyncFromHost()
        {
            assert(m_DeviceBuffer.GetData() != nullptr);
            return m_DeviceBuffer.UploadSync(&m_HostValue, 1);
        }

        T* GetDevicePointer() const { return reinterpret_cast<T*>(m_DeviceBuffer.GetData()); }
        const DeviceBuffer1D& GetDeviceBuffer() const { return m_DeviceBuffer; }

        T GetHostValue() const { return m_HostValue; }
        void SetHostValue(T value) { m_HostValue = value; }

        CounterView<T> GetView() const
        {
            return { reinterpret_cast<T*>(m_DeviceBuffer.GetData()) };
        }
    private:
        DeviceBuffer1D m_DeviceBuffer;
        T m_HostValue = 0;
    };
}