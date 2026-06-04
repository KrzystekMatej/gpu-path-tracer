#include <Core/Graphics/Cuda/Runtime/DeviceBuffer1D.hpp>
#include <Core/Graphics/Cuda/Utils/Error.hpp>
#include <cuda_runtime.h>
#include <utility>

namespace Core::Graphics::Cuda::Runtime
{
    DeviceBuffer1D::~DeviceBuffer1D()
    {
        (void)Free();
    }

    DeviceBuffer1D::DeviceBuffer1D(DeviceBuffer1D&& other) noexcept
        : m_Data(std::exchange(other.m_Data, nullptr)),
        m_Size(other.m_Size),
        m_ElementSize(other.m_ElementSize)
    {
    }

    DeviceBuffer1D& DeviceBuffer1D::operator=(DeviceBuffer1D&& other) noexcept
    {
        if (this != &other)
        {
            (void)Free();
            m_Data = std::exchange(other.m_Data, nullptr);
            m_Size = other.m_Size;
            m_ElementSize = other.m_ElementSize;
        }

        return *this;
    }

    std::expected<void, Core::Utils::Error> DeviceBuffer1D::Allocate(uint32_t size, uint32_t elementSize, const Stream& stream)
    {
        CORE_TRY_DISCARD(Free(stream));
        
        void* data = nullptr;
        CUDA_TRY("cudaMalloc", cudaMallocAsync(&data, size * elementSize, stream.GetRawHandle()));

        m_Data = data;
        m_Size = size;
        m_ElementSize = elementSize;
        return {};
    }

    std::expected<void, Core::Utils::Error> DeviceBuffer1D::Upload(const void* hostData, uint32_t size, const Stream& stream) const
    {
        assert(m_Data != nullptr);
        assert(hostData != nullptr);
        assert(size <= m_Size);

        CUDA_TRY("cudaMemcpyAsync", cudaMemcpyAsync(
            m_Data,
            hostData,
            size * m_ElementSize,
            cudaMemcpyKind::cudaMemcpyHostToDevice,
            stream.GetRawHandle()));
        return {};

    }

    std::expected<void, Core::Utils::Error> DeviceBuffer1D::MemsetBytes(uint8_t value, const Stream& stream) const
    {
        assert(m_Data != nullptr);

        CUDA_TRY("cudaMemsetAsync", cudaMemsetAsync(m_Data, value, m_Size * m_ElementSize, stream.GetRawHandle()));
        return {};
    }

    std::expected<void, Core::Utils::Error> DeviceBuffer1D::Free(const Stream& stream)
    {
        if (m_Data == nullptr)
            return {};

        CUDA_TRY("cudaFree", cudaFreeAsync(m_Data, stream.GetRawHandle()));
        m_Data = nullptr;
        m_Size = 0;
        m_ElementSize = 0;
        return {};
    }
}
