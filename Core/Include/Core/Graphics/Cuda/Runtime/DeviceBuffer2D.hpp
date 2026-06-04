#pragma once
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <expected>
#include <type_traits>
#include <Core/Graphics/Cuda/Runtime/DeviceBuffer2DView.hpp>
#include <Core/Utils/Error.hpp>
#include <Core/Graphics/Cuda/Runtime/Stream.hpp>

namespace Core::Graphics::Cuda::Runtime
{
    class DeviceBuffer2D
    {
    public:
        DeviceBuffer2D() = default;
        ~DeviceBuffer2D();

        DeviceBuffer2D(DeviceBuffer2D&& other) noexcept;
        DeviceBuffer2D& operator=(DeviceBuffer2D&& other) noexcept;

        DeviceBuffer2D(const DeviceBuffer2D&) = delete;
        DeviceBuffer2D& operator=(const DeviceBuffer2D&) = delete;

        std::expected<void, Core::Utils::Error> Allocate(uint32_t width, uint32_t height, uint32_t elementSize);
        std::expected<void, Core::Utils::Error> Upload(const void* hostData, uint32_t hostPitchElement, const Stream& stream = Stream::Default()) const;
        std::expected<void, Core::Utils::Error> MemsetBytes(uint8_t value, const Stream& stream = Stream::Default()) const;
        std::expected<void, Core::Utils::Error> Free(const Stream& stream = Stream::Default());

        void* GetData() const { return m_Data; }
        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
        uint32_t GetPitchBytes() const { return m_PitchBytes; }
        uint32_t GetElementSize() const { return m_ElementSize; }

        template<typename T>
        DeviceBuffer2DView<T> GetView() const
        {
            static_assert(!std::is_void_v<T>);
            assert(m_ElementSize == sizeof(T));

            return DeviceBuffer2DView<T>(reinterpret_cast<T*>(m_Data), m_Width, m_Height, m_PitchBytes);
        }

    private:
        void* m_Data = nullptr;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        uint32_t m_PitchBytes = 0;
        uint32_t m_ElementSize = 0;
    };
}
