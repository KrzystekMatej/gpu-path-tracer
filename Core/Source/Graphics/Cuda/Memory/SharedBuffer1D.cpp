#include <Core/Graphics/Cuda/Memory/SharedBuffer1D.hpp>
#include <Core/Graphics/Cuda/Utils/Error.hpp>
#include <cuda_runtime.h>
#include <utility>

namespace Core::Graphics::Cuda::Memory
{
    SharedBuffer1D::~SharedBuffer1D()
    {
        Free();
    }

    SharedBuffer1D::SharedBuffer1D(SharedBuffer1D&& other) noexcept
        : m_HostData(std::exchange(other.m_HostData, nullptr))
        , m_DeviceBuffer(std::move(other.m_DeviceBuffer))
    {
    }

    SharedBuffer1D& SharedBuffer1D::operator=(SharedBuffer1D&& other) noexcept
    {
        if (this != &other)
        {
            Free();
            m_HostData = std::exchange(other.m_HostData, nullptr);
            m_DeviceBuffer = std::move(other.m_DeviceBuffer);
        }

        return *this;
    }

    std::expected<void, Core::Utils::Error> SharedBuffer1D::Allocate(size_t size, size_t elementSize)
    {
        CORE_TRY_DISCARD(Free());

        void* hostData = nullptr;
        CUDA_TRY("cudaMallocHost", cudaMallocHost(&hostData, size * elementSize));

        auto deviceAllocateResult = m_DeviceBuffer.Allocate(size, elementSize);
        if (!deviceAllocateResult)
        {
            cudaFreeHost(hostData);
            return std::unexpected(deviceAllocateResult.error());
        }

        m_HostData = hostData;
        return {};
    }

    std::expected<void, Core::Utils::Error> SharedBuffer1D::CopyHostToDeviceSync() const
    {
        assert(m_HostData != nullptr);
        return m_DeviceBuffer.UploadSync(m_HostData, m_DeviceBuffer.GetSize());
    }

    std::expected<void, Core::Utils::Error> SharedBuffer1D::CopyHostToDeviceAsync(void* stream) const
    {
        assert(m_HostData != nullptr);
        return m_DeviceBuffer.UploadAsync(m_HostData, m_DeviceBuffer.GetSize(), stream);
    }

    std::expected<void, Core::Utils::Error> SharedBuffer1D::CopyDeviceToHostSync() const
    {
        assert(m_HostData != nullptr);

        CUDA_TRY("cudaMemcpy", cudaMemcpy(
            m_HostData,
            m_DeviceBuffer.GetData(),
            m_DeviceBuffer.GetSize() * m_DeviceBuffer.GetElementSize(),
            cudaMemcpyKind::cudaMemcpyDeviceToHost));

        return {};
    }

    std::expected<void, Core::Utils::Error> SharedBuffer1D::CopyDeviceToHostAsync(void* stream) const
    {
        assert(m_HostData != nullptr);
        assert(stream != nullptr);

        CUDA_TRY("cudaMemcpyAsync", cudaMemcpyAsync(
            m_HostData,
            m_DeviceBuffer.GetData(),
            m_DeviceBuffer.GetSize() * m_DeviceBuffer.GetElementSize(),
            cudaMemcpyKind::cudaMemcpyDeviceToHost,
            static_cast<cudaStream_t>(stream)));

        return {};
    }

	std::expected<void, Core::Utils::Error> SharedBuffer1D::Free()
    {
        cudaError_t hostError = cudaSuccess;

        if (m_HostData != nullptr)
        {
            hostError = cudaFreeHost(m_HostData);
            m_HostData = nullptr;
        }

        auto deviceFreeResult = m_DeviceBuffer.Free();

        if (hostError != cudaSuccess)
            return std::unexpected(Utils::MakeCudaError("cudaFreeHost", hostError));

        if (!deviceFreeResult)
            return std::unexpected(std::move(deviceFreeResult).error());

        return {};
    }
}
