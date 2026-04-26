#pragma once
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <expected>
#include <type_traits>
#include <Core/Graphics/Cuda/Memory/DeviceBuffer2DView.hpp>
#include <Core/Utils/Error.hpp>

namespace Core::Graphics::Cuda::Memory
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

        std::expected<void, Core::Utils::Error> Allocate(size_t width, size_t height, size_t elementSize);
        std::expected<void, Core::Utils::Error> UploadSync(const void* hostData, size_t hostPitchElement) const;
        std::expected<void, Core::Utils::Error> UploadAsync(const void* hostData, size_t hostPitchElement, void* stream) const;
        std::expected<void, Core::Utils::Error> MemsetBytesSync(uint8_t value = 0) const;
        std::expected<void, Core::Utils::Error> MemsetBytesAsync(uint8_t value, void* stream) const;
        std::expected<void, Core::Utils::Error> Free();

        void* GetData() const { return m_Data; }
        size_t GetWidth() const { return m_Width; }
        size_t GetHeight() const { return m_Height; }
        size_t GetPitchBytes() const { return m_PitchBytes; }
        size_t GetElementSize() const { return m_ElementSize; }

        template<typename T>
        DeviceBuffer2DView<T> GetView() const
        {
            static_assert(!std::is_void_v<T>);
            assert(m_ElementSize == sizeof(T));

            return
            {
                reinterpret_cast<T*>(m_Data),
                m_Width,
                m_Height,
                m_PitchBytes
            };
        }

    private:
        void ResetState() noexcept;

        void* m_Data = nullptr;
        size_t m_Width = 0;
        size_t m_Height = 0;
        size_t m_PitchBytes = 0;
        size_t m_ElementSize = 0;
    };
}
