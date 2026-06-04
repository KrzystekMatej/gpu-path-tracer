#include <Core/Graphics/Cuda/Runtime/SharedBuffer2D.hpp>
#include <Core/Graphics/Cuda/Utils/Error.hpp>
#include <cuda_runtime.h>
#include <utility>

namespace Core::Graphics::Cuda::Runtime
{
    SharedBuffer2D::~SharedBuffer2D()
    {
        (void)Free();
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
            (void)Free();
            m_HostData = std::exchange(other.m_HostData, nullptr);
            m_DeviceBuffer = std::move(other.m_DeviceBuffer);
        }

        return *this;
    }

    std::expected<void, Core::Utils::Error> SharedBuffer2D::Allocate(uint32_t width, uint32_t height, uint32_t elementSize)
    {
        CORE_TRY_DISCARD(Free());

        const uint32_t rowSize = width * elementSize;

        void* hostData = nullptr;
        CUDA_TRY("cudaMallocHost", cudaMallocHost(&hostData, rowSize * height));

        auto deviceAllocateResult = m_DeviceBuffer.Allocate(width, height, elementSize);
        if (!deviceAllocateResult)
        {
            cudaFreeHost(hostData);
            return std::unexpected(deviceAllocateResult.error());
        }

        m_HostData = hostData;
        return {};
    }

    std::expected<void, Core::Utils::Error> SharedBuffer2D::CopyHostToDevice(const Stream& stream) const
    {
        assert(m_HostData != nullptr);
        return m_DeviceBuffer.Upload(m_HostData, m_DeviceBuffer.GetWidth(), stream);
    }

    std::expected<void, Core::Utils::Error> SharedBuffer2D::CopyDeviceToHost(const Stream& stream) const
    {
        assert(m_HostData != nullptr);

        const size_t rowSize = m_DeviceBuffer.GetWidth() * m_DeviceBuffer.GetElementSize();

        CUDA_TRY("cudaMemcpy2DAsync", cudaMemcpy2DAsync(
            m_HostData,
            rowSize,
            m_DeviceBuffer.GetData(),
            m_DeviceBuffer.GetPitchBytes(),
            rowSize,
            m_DeviceBuffer.GetHeight(),
            cudaMemcpyKind::cudaMemcpyDeviceToHost,
            stream.GetRawHandle()));

        return {};
    }

	std::expected<void, Core::Utils::Error> SharedBuffer2D::Free(const Stream& stream)
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
			return std::unexpected(deviceFreeResult.error());

		return {};
	}
}
