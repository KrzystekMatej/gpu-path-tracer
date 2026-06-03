#pragma once
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <expected>
#include <type_traits>
#include <Core/Graphics/Cuda/Memory/DeviceBuffer1DView.hpp>
#include <Core/Utils/Error.hpp>
#include <Core/Graphics/Cuda/Memory/Stream.hpp>

namespace Core::Graphics::Cuda::Memory
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

        std::expected<void, Core::Utils::Error> Allocate(uint32_t size, uint32_t elementSize);
        std::expected<void, Core::Utils::Error> UploadSync(const void* hostData, uint32_t size) const;
        std::expected<void, Core::Utils::Error> UploadAsync(const void* hostData, uint32_t size, const Stream& stream) const;
        std::expected<void, Core::Utils::Error> MemsetBytesSync(uint8_t value = 0) const;
        std::expected<void, Core::Utils::Error> MemsetBytesAsync(uint8_t value, const Stream& stream) const;
        std::expected<void, Core::Utils::Error> Free();

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
