#pragma once
#include <cstddef>
#include <cstdint>
#include <expected>
#include <type_traits>
#include <cassert>
#include <Core/Graphics/Cuda/Memory/DeviceBuffer1D.hpp>
#include <Core/Graphics/Cuda/Memory/DeviceBuffer1DView.hpp>
#include <Core/Utils/Error.hpp>

namespace Core::Graphics::Cuda::Memory
{
    class SharedBuffer1D
    {
    public:
        SharedBuffer1D() = default;
        ~SharedBuffer1D();

        SharedBuffer1D(const SharedBuffer1D&) = delete;
        SharedBuffer1D& operator=(const SharedBuffer1D&) = delete;

        SharedBuffer1D(SharedBuffer1D&& other) noexcept;
        SharedBuffer1D& operator=(SharedBuffer1D&& other) noexcept;

        std::expected<void, Core::Utils::Error> Allocate(size_t size, size_t elementSize);
        std::expected<void, Core::Utils::Error> CopyHostToDeviceSync() const;
        std::expected<void, Core::Utils::Error> CopyHostToDeviceAsync(void* stream) const;
        std::expected<void, Core::Utils::Error> CopyDeviceToHostSync() const;
        std::expected<void, Core::Utils::Error> CopyDeviceToHostAsync(void* stream) const;
        std::expected<void, Core::Utils::Error> Free();

        void* GetHostData() const { return m_HostData; }
        DeviceBuffer1D& GetDeviceBuffer() { return m_DeviceBuffer; }
        const DeviceBuffer1D& GetDeviceBuffer() const { return m_DeviceBuffer; }

        size_t GetSize() const { return m_DeviceBuffer.GetSize(); }
        size_t GetElementSize() const { return m_DeviceBuffer.GetElementSize(); }

        template<typename T>
        T* GetHost() const
        {
            static_assert(!std::is_void_v<T>);
            assert(m_DeviceBuffer.GetElementSize() == sizeof(T));
            return reinterpret_cast<T*>(m_HostData);
        }

        template<typename T>
        DeviceBuffer1DView<T> GetDeviceView() const
        {
            return m_DeviceBuffer.GetView<T>();
        }

    private:
        void* m_HostData = nullptr;
        DeviceBuffer1D m_DeviceBuffer;
    };
}
