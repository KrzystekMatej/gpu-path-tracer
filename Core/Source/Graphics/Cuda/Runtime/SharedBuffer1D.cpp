#include <Core/Graphics/Cuda/Runtime/SharedBuffer1D.hpp>
#include <Core/Graphics/Cuda/Utils/Error.hpp>
#include <cuda_runtime.h>
#include <utility>

namespace Core::Graphics::Cuda::Runtime
{
    SharedBuffer1D::~SharedBuffer1D()
    {
        (void)Free();
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
            (void)Free();
            m_HostData = std::exchange(other.m_HostData, nullptr);
            m_DeviceBuffer = std::move(other.m_DeviceBuffer);
        }

        return *this;
    }

    std::expected<void, Core::Utils::Error> SharedBuffer1D::Allocate(uint32_t size, uint32_t elementSize, const Stream& stream)
    {
        CORE_TRY_DISCARD(Free(stream));

        void* hostData = nullptr;
        CUDA_TRY("cudaMallocHost", cudaMallocHost(&hostData, size * elementSize));

        auto deviceAllocateResult = m_DeviceBuffer.Allocate(size, elementSize, stream);
        if (!deviceAllocateResult)
        {
            cudaFreeHost(hostData);
            return std::unexpected(deviceAllocateResult.error());
        }

        m_HostData = hostData;
        return {};
    }


    std::expected<void, Core::Utils::Error> SharedBuffer1D::CopyHostToDevice(const Stream& stream) const
    {
        assert(m_HostData != nullptr);
        return m_DeviceBuffer.Upload(m_HostData, m_DeviceBuffer.GetSize(), stream);
    }

    std::expected<void, Core::Utils::Error> SharedBuffer1D::CopyDeviceToHost(const Stream& stream) const
    {
        assert(m_HostData != nullptr);

        CUDA_TRY("cudaMemcpyAsync", cudaMemcpyAsync(
            m_HostData,
            m_DeviceBuffer.GetData(),
            m_DeviceBuffer.GetSize() * m_DeviceBuffer.GetElementSize(),
            cudaMemcpyKind::cudaMemcpyDeviceToHost,
            stream.GetRawHandle()));

        return {};
    }

	std::expected<void, Core::Utils::Error> SharedBuffer1D::Free(const Stream& stream)
    {
        cudaError_t hostError = cudaSuccess;

        if (m_HostData != nullptr)
        {
            hostError = cudaFreeHost(m_HostData);
            m_HostData = nullptr;
        }

        auto deviceFreeResult = m_DeviceBuffer.Free(stream);

        if (hostError != cudaSuccess)
            return std::unexpected(Utils::MakeCudaError("cudaFreeHost", hostError));

        if (!deviceFreeResult)
            return std::unexpected(std::move(deviceFreeResult).error());

        return {};
    }
}
