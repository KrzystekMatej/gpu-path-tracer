#pragma once
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <expected>
#include <type_traits>
#include <Core/Graphics/Cuda/Runtime/DeviceBuffer1DView.hpp>
#include <Core/Utils/Error.hpp>
#include <Core/Graphics/Cuda/Runtime/Stream.hpp>

namespace Core::Graphics::Cuda::Runtime
{
    class DeviceBuffer1D
    {
    public:
        DeviceBuffer1D() = default;
        ~DeviceBuffer1D();

        DeviceBuffer1D(DeviceBuffer1D&& other) noexcept;
        DeviceBuffer1D& operator=(DeviceBuffer1D&& other) noexcept;

        DeviceBuffer1D(const DeviceBuffer1D&) = delete;
        DeviceBuffer1D& operator=(const DeviceBuffer1D&) = delete;

        std::expected<void, Core::Utils::Error> Allocate(uint32_t size, uint32_t elementSize, const Stream& stream = Stream::Default());
        std::expected<void, Core::Utils::Error> Upload(const void* hostData, uint32_t size, const Stream& stream = Stream::Default()) const;
        std::expected<void, Core::Utils::Error> MemsetBytes(uint8_t value, const Stream& stream = Stream::Default()) const;
        std::expected<void, Core::Utils::Error> Free(const Stream& stream = Stream::Default());

        void* GetData() const { return m_Data; }
        uint32_t GetSize() const { return m_Size; }
        uint32_t GetElementSize() const { return m_ElementSize; }

        template<typename T>
        DeviceBuffer1DView<T> GetView() const
        {
            static_assert(!std::is_void_v<T>);
            assert(m_ElementSize == sizeof(T));

            return DeviceBuffer1DView<T>(reinterpret_cast<T*>(m_Data), m_Size);
        }

    private:
        void* m_Data = nullptr;
        uint32_t m_Size = 0;
        uint32_t m_ElementSize = 0;
    };
}
