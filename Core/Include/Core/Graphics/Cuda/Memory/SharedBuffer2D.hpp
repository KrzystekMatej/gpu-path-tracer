#pragma once
#include <cstddef>
#include <cstdint>
#include <expected>
#include <type_traits>
#include <cassert>
#include <Core/Graphics/Cuda/Memory/DeviceBuffer2D.hpp>
#include <Core/Graphics/Cuda/Memory/DeviceBuffer2DView.hpp>
#include <Core/Utils/Error.hpp>

namespace Core::Graphics::Cuda::Memory
{
    class SharedBuffer2D
    {
    public:
        SharedBuffer2D() = default;
        ~SharedBuffer2D();

        SharedBuffer2D(const SharedBuffer2D&) = delete;
        SharedBuffer2D& operator=(const SharedBuffer2D&) = delete;

        SharedBuffer2D(SharedBuffer2D&& other) noexcept;
        SharedBuffer2D& operator=(SharedBuffer2D&& other) noexcept;

        std::expected<void, Core::Utils::Error> Allocate(size_t width, size_t height, size_t elementSize);
        std::expected<void, Core::Utils::Error> CopyHostToDeviceSync() const;
        std::expected<void, Core::Utils::Error> CopyHostToDeviceAsync(void* stream) const;
        std::expected<void, Core::Utils::Error> CopyDeviceToHostSync() const;
        std::expected<void, Core::Utils::Error> CopyDeviceToHostAsync(void* stream) const;
        std::expected<void, Core::Utils::Error> Free();

        void* GetHostData() const { return m_HostData; }
        DeviceBuffer2D& GetDeviceBuffer() { return m_DeviceBuffer; }
        const DeviceBuffer2D& GetDeviceBuffer() const { return m_DeviceBuffer; }

        size_t GetWidth() const { return m_DeviceBuffer.GetWidth(); }
        size_t GetHeight() const { return m_DeviceBuffer.GetHeight(); }
        size_t GetElementSize() const { return m_DeviceBuffer.GetElementSize(); }

        template<typename T>
        T* GetHost() const
        {
            static_assert(!std::is_void_v<T>);
            assert(m_DeviceBuffer.GetElementSize() == sizeof(T));
            return reinterpret_cast<T*>(m_HostData);
        }

        template<typename T>
        DeviceBuffer2DView<T> GetDeviceView() const
        {
            return m_DeviceBuffer.GetView<T>();
        }

    private:
        void* m_HostData = nullptr;
        DeviceBuffer2D m_DeviceBuffer;
    };
}
