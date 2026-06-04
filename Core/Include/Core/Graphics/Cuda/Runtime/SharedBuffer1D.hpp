#pragma once
#include <cstddef>
#include <cstdint>
#include <expected>
#include <type_traits>
#include <cassert>
#include <Core/Graphics/Cuda/Runtime/DeviceBuffer1D.hpp>
#include <Core/Graphics/Cuda/Runtime/DeviceBuffer1DView.hpp>
#include <Core/Utils/Error.hpp>
#include <Core/Graphics/Cuda/Runtime/Stream.hpp>

namespace Core::Graphics::Cuda::Runtime
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

        std::expected<void, Core::Utils::Error> Allocate(uint32_t size, uint32_t elementSize, const Stream& stream = Stream::Default());
        std::expected<void, Core::Utils::Error> CopyHostToDevice(const Stream& stream = Stream::Default()) const;
        std::expected<void, Core::Utils::Error> CopyDeviceToHost(const Stream& stream = Stream::Default()) const;
        std::expected<void, Core::Utils::Error> Free(const Stream& stream = Stream::Default());

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
