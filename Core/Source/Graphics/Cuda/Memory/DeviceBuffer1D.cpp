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

    void DeviceBuffer1D::ResetState() noexcept
    {
        m_Data = nullptr;
        m_Size = 0;
        m_ElementSize = 0;
    }

    std::expected<void, Core::Utils::Error> DeviceBuffer1D::Allocate(size_t size, size_t elementSize)
    {
        auto freeResult = Free();
        if (!freeResult)
            return std::unexpected(freeResult.error());

        void* data = nullptr;
        cudaError_t error = cudaMalloc(&data, size);
        if (error != cudaSuccess)
            return std::unexpected(Utils::MakeCudaError("cudaMalloc", error));

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

        cudaError_t error = cudaMemcpy(m_Data, hostData, size, cudaMemcpyHostToDevice);
        if (error != cudaSuccess)
            return std::unexpected(Utils::MakeCudaError("cudaMemcpy", error));

        return {};
    }

    std::expected<void, Core::Utils::Error> DeviceBuffer1D::UploadAsync(const void* hostData, size_t size, void* stream) const
    {
        assert(m_Data != nullptr);
        assert(hostData != nullptr);
        assert(stream != nullptr);
        assert(size <= m_Size);

        cudaError_t error = cudaMemcpyAsync(
            m_Data,
            hostData,
            size,
            cudaMemcpyHostToDevice,
            static_cast<cudaStream_t>(stream));

        if (error != cudaSuccess)
            return std::unexpected(Utils::MakeCudaError("cudaMemcpyAsync", error));

        return {};
    }

    std::expected<void, Core::Utils::Error> DeviceBuffer1D::MemsetSync(int value) const
    {
        assert(m_Data != nullptr);

        cudaError_t error = cudaMemset(m_Data, value, m_Size);
        if (error != cudaSuccess)
            return std::unexpected(Utils::MakeCudaError("cudaMemset", error));

        return {};
    }

    std::expected<void, Core::Utils::Error> DeviceBuffer1D::MemsetAsync(int value, void* stream) const
    {
        assert(m_Data != nullptr);
        assert(stream != nullptr);

        cudaError_t error = cudaMemsetAsync(m_Data, value, m_Size, static_cast<cudaStream_t>(stream));
        if (error != cudaSuccess)
            return std::unexpected(Utils::MakeCudaError("cudaMemsetAsync", error));

        return {};
    }

    std::expected<void, Core::Utils::Error> DeviceBuffer1D::Free()
    {
        if (m_Data == nullptr)
            return {};

        cudaError_t error = cudaFree(m_Data);
        if (error != cudaSuccess)
            return std::unexpected(Utils::MakeCudaError("cudaFree", error));

        ResetState();
        return {};
    }
}
