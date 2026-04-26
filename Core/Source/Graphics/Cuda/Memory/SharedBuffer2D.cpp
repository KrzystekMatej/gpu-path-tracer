#include <Core/Graphics/Cuda/Memory/SharedBuffer2D.hpp>
#include <Core/Graphics/Cuda/Utils/Error.hpp>
#include <cuda_runtime.h>
#include <utility>

namespace Core::Graphics::Cuda::Memory
{
    SharedBuffer2D::~SharedBuffer2D()
    {
        Free();
    }

    SharedBuffer2D::SharedBuffer2D(SharedBuffer2D&& other) noexcept
        : m_HostData(std::exchange(other.m_HostData, nullptr))
        , m_DeviceBuffer(std::move(other.m_DeviceBuffer))
    {
    }

    SharedBuffer2D& SharedBuffer2D::operator=(SharedBuffer2D&& other) noexcept
    {
        if (this != &other)
        {
            Free();
            m_HostData = std::exchange(other.m_HostData, nullptr);
            m_DeviceBuffer = std::move(other.m_DeviceBuffer);
        }

        return *this;
    }

    std::expected<void, Core::Utils::Error> SharedBuffer2D::Allocate(size_t width, size_t height, size_t elementSize)
    {
        auto freeResult = Free();
        if (!freeResult)
            return std::unexpected(freeResult.error());

        const size_t rowSize = width * elementSize;

        void* hostData = nullptr;
        cudaError_t error = cudaMallocHost(&hostData, rowSize * height);
        if (error != cudaSuccess)
            return std::unexpected(Utils::MakeCudaError("cudaMallocHost", error));

        auto deviceAllocateResult = m_DeviceBuffer.Allocate(width, height, elementSize);
        if (!deviceAllocateResult)
        {
            cudaFreeHost(hostData);
            return std::unexpected(deviceAllocateResult.error());
        }

        m_HostData = hostData;
        return {};
    }

    std::expected<void, Core::Utils::Error> SharedBuffer2D::CopyHostToDeviceSync() const
    {
        assert(m_HostData != nullptr);
        return m_DeviceBuffer.UploadSync(m_HostData, m_DeviceBuffer.GetWidth());
    }

    std::expected<void, Core::Utils::Error> SharedBuffer2D::CopyHostToDeviceAsync(void* stream) const
    {
        assert(m_HostData != nullptr);
        return m_DeviceBuffer.UploadAsync(m_HostData, m_DeviceBuffer.GetWidth(), stream);
    }

    std::expected<void, Core::Utils::Error> SharedBuffer2D::CopyDeviceToHostSync() const
    {
        assert(m_HostData != nullptr);

        const size_t rowSize = m_DeviceBuffer.GetWidth() * m_DeviceBuffer.GetElementSize();

        cudaError_t error = cudaMemcpy2D(
            m_HostData,
            rowSize,
            m_DeviceBuffer.GetData(),
            m_DeviceBuffer.GetPitchBytes(),
            rowSize,
            m_DeviceBuffer.GetHeight(),
            cudaMemcpyDeviceToHost);

        if (error != cudaSuccess)
            return std::unexpected(Utils::MakeCudaError("cudaMemcpy2D", error));

        return {};
    }

    std::expected<void, Core::Utils::Error> SharedBuffer2D::CopyDeviceToHostAsync(void* stream) const
    {
        assert(m_HostData != nullptr);
        assert(stream != nullptr);

        const size_t rowSize = m_DeviceBuffer.GetWidth() * m_DeviceBuffer.GetElementSize();

        cudaError_t error = cudaMemcpy2DAsync(
            m_HostData,
            rowSize,
            m_DeviceBuffer.GetData(),
            m_DeviceBuffer.GetPitchBytes(),
            rowSize,
            m_DeviceBuffer.GetHeight(),
            cudaMemcpyDeviceToHost,
            static_cast<cudaStream_t>(stream));

        if (error != cudaSuccess)
            return std::unexpected(Utils::MakeCudaError("cudaMemcpy2DAsync", error));

        return {};
    }

	std::expected<void, Core::Utils::Error> SharedBuffer2D::Free()
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
			return std::unexpected(deviceFreeResult.error());

		return {};
	}
}
