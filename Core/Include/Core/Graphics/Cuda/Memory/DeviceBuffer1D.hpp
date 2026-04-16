#pragma once
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <expected>
#include <type_traits>
#include <Core/Graphics/Cuda/Memory/DeviceBuffer1DView.hpp>
#include <Core/Utils/Error.hpp>

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

        std::expected<void, Core::Utils::Error> Allocate(size_t size, size_t elementSize);
        std::expected<void, Core::Utils::Error> UploadSync(const void* hostData, size_t size) const;
        std::expected<void, Core::Utils::Error> UploadAsync(const void* hostData, size_t size, void* stream) const;
        std::expected<void, Core::Utils::Error> MemsetSync(int value = 0) const;
        std::expected<void, Core::Utils::Error> MemsetAsync(int value, void* stream) const;
        std::expected<void, Core::Utils::Error> Free();

        void* GetData() const { return m_Data; }
        size_t GetSize() const { return m_Size; }
        size_t GetElementSize() const { return m_ElementSize; }

        template<typename T>
        DeviceBuffer1DView<T> GetView() const
        {
            static_assert(!std::is_void_v<T>);
            assert(m_ElementSize == sizeof(T));

            return
            {
                reinterpret_cast<T*>(m_Data),
                m_Size / sizeof(T)
            };
        }

    private:
        void ResetState() noexcept;

        void* m_Data = nullptr;
        size_t m_Size = 0;
        size_t m_ElementSize = 0;
    };
}
