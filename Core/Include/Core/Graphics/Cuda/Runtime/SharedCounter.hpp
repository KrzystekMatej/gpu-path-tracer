#pragma once

#include <cassert>
#include <cstdint>
#include <expected>
#include <type_traits>

#include <Core/Graphics/Cuda/Runtime/CounterView.hpp>
#include <Core/Graphics/Cuda/Runtime/DeviceBuffer1D.hpp>
#include <Core/Graphics/Cuda/Runtime/SharedBuffer1D.hpp>
#include <Core/Graphics/Cuda/Runtime/Stream.hpp>
#include <Core/Graphics/Cuda/Utils/Error.hpp>
#include <Core/Utils/Error.hpp>

namespace Core::Graphics::Cuda::Runtime
{
    template<typename T>
    class SharedCounter
    {
    public:
        static_assert(std::is_trivially_copyable_v<T>);

        SharedCounter() = default;
        ~SharedCounter() = default;

        SharedCounter(const SharedCounter&) = delete;
        SharedCounter& operator=(const SharedCounter&) = delete;

        SharedCounter(SharedCounter&& other) noexcept = default;
        SharedCounter& operator=(SharedCounter&& other) noexcept = default;

        std::expected<void, Core::Utils::Error> Allocate(const Stream& stream = Stream::Default())
        {
            CORE_TRY_DISCARD(Free(stream));
            CORE_TRY_DISCARD(m_Buffer.Allocate(1, sizeof(T), stream));
            return Reset(stream);
        }

        std::expected<void, Core::Utils::Error> Free(const Stream& stream = Stream::Default())
        {
            return m_Buffer.Free(stream);
        }

        std::expected<void, Core::Utils::Error> Reset(const Stream& stream = Stream::Default())
        {
            SetHostValue(T{});
            return CopyHostToDeviceAsync(stream);
        }

        std::expected<void, Core::Utils::Error> CopyDeviceToHostAsync(const Stream& stream = Stream::Default())
        {
            assert(m_Buffer.GetHostData() != nullptr);
            assert(m_Buffer.GetDeviceBuffer().GetData() != nullptr);

            return m_Buffer.CopyDeviceToHost(stream);
        }

        std::expected<void, Core::Utils::Error> CopyHostToDeviceAsync(const Stream& stream = Stream::Default()) const
        {
            assert(m_Buffer.GetHostData() != nullptr);
            assert(m_Buffer.GetDeviceBuffer().GetData() != nullptr);

            return m_Buffer.CopyHostToDevice(stream);
        }

        std::expected<T, Core::Utils::Error> SyncFromDevice(const Stream& stream = Stream::Default())
        {
            CORE_TRY_DISCARD(CopyDeviceToHostAsync(stream));
            return Synchronize(stream);
        }

        std::expected<T, Core::Utils::Error> SyncFromHost(const Stream& stream = Stream::Default()) const
        {
            CORE_TRY_DISCARD(CopyHostToDeviceAsync(stream));
            CORE_TRY_DISCARD(stream.Synchronize());
            return GetHostValue();
        }

        std::expected<T, Core::Utils::Error> Synchronize(const Stream& stream = Stream::Default()) const
        {
            CORE_TRY_DISCARD(stream.Synchronize());
            return GetHostValue();
        }

        T* GetDevicePointer() const
        {
            return reinterpret_cast<T*>(m_Buffer.GetDeviceBuffer().GetData());
        }

        T* GetHostPointer() const
        {
            assert(m_Buffer.GetHostData() != nullptr);
            return m_Buffer.GetHost<T>();
        }

        const DeviceBuffer1D& GetDeviceBuffer() const
        {
            return m_Buffer.GetDeviceBuffer();
        }

        const SharedBuffer1D& GetSharedBuffer() const
        {
            return m_Buffer;
        }

        SharedBuffer1D& GetSharedBuffer()
        {
            return m_Buffer;
        }

        T GetHostValue() const
        {
            assert(m_Buffer.GetHostData() != nullptr);
            return *m_Buffer.GetHost<T>();
        }

        void SetHostValue(T value)
        {
            assert(m_Buffer.GetHostData() != nullptr);
            *m_Buffer.GetHost<T>() = value;
        }

        CounterView<T> GetView() const
        {
            return { GetDevicePointer() };
        }

    private:
        SharedBuffer1D m_Buffer;
    };
}