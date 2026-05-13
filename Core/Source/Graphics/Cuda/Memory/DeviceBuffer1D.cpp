#include <Core/Graphics/Cuda/Memory/DeviceBuffer1D.hpp>
#include <Core/Graphics/Cuda/Utils/Error.hpp>
#include <cuda_runtime.h>
#include <utility>

namespace Core::Graphics::Cuda::Memory
{
    DeviceBuffer1D::~DeviceBuffer1D()
    {
        Free();
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
            Free();
            m_Data = std::exchange(other.m_Data, nullptr);
            m_Size = other.m_Size;
            m_ElementSize = other.m_ElementSize;
        }

        return *this;
    }

    std::expected<void, Core::Utils::Error> DeviceBuffer1D::Allocate(size_t size, size_t elementSize)
    {
        CORE_TRY_DISCARD(Free());
        
        void* data = nullptr;
        CORE_CUDA_TRY("cudaMalloc", cudaMalloc(&data, size * elementSize));

        m_Data = data;
        m_Size = size;
        m_ElementSize = elementSize;
        return {};
    }

    std::expected<void, Core::Utils::Error> DeviceBuffer1D::UploadSync(const void* hostData, size_t size) const
    {
        assert(m_Data != nullptr);
        assert(hostData != nullptr);
        assert(size <= m_Size);

        CORE_CUDA_TRY("cudaMemcpy", cudaMemcpy(m_Data, hostData, size * m_ElementSize, cudaMemcpyKind::cudaMemcpyHostToDevice));
        return {};
    }

    std::expected<void, Core::Utils::Error> DeviceBuffer1D::UploadAsync(const void* hostData, size_t size, void* stream) const
    {
        assert(m_Data != nullptr);
        assert(hostData != nullptr);
        assert(stream != nullptr);
        assert(size <= m_Size);

        CORE_CUDA_TRY("cudaMemcpyAsync", cudaMemcpyAsync(
            m_Data,
            hostData,
            size * m_ElementSize,
            cudaMemcpyKind::cudaMemcpyHostToDevice,
            static_cast<cudaStream_t>(stream)));
        return {};

    }

    std::expected<void, Core::Utils::Error> DeviceBuffer1D::MemsetBytesSync(uint8_t value) const
    {
        assert(m_Data != nullptr);

        CORE_CUDA_TRY("cudaMemset", cudaMemset(m_Data, value, m_Size * m_ElementSize));
        return {};
    }

    std::expected<void, Core::Utils::Error> DeviceBuffer1D::MemsetBytesAsync(uint8_t value, void* stream) const
    {
        assert(m_Data != nullptr);
        assert(stream != nullptr);

        CORE_CUDA_TRY("cudaMemsetAsync", cudaMemsetAsync(m_Data, value, m_Size * m_ElementSize, static_cast<cudaStream_t>(stream)));
        return {};
    }

    std::expected<void, Core::Utils::Error> DeviceBuffer1D::Free()
    {
        if (m_Data == nullptr)
            return {};

        CORE_CUDA_TRY("cudaFree", cudaFree(m_Data));
        m_Data = nullptr;
        m_Size = 0;
        m_ElementSize = 0;
        return {};
    }
}
