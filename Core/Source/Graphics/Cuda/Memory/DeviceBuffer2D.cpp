#include <Core/Graphics/Cuda/Memory/DeviceBuffer2D.hpp>
#include <Core/Graphics/Cuda/Utils/Error.hpp>
#include <cuda_runtime.h>
#include <utility>

namespace Core::Graphics::Cuda::Memory
{
    DeviceBuffer2D::~DeviceBuffer2D()
    {
        Free();
    }

    DeviceBuffer2D::DeviceBuffer2D(DeviceBuffer2D&& other) noexcept
        : m_Data(std::exchange(other.m_Data, nullptr)),
        m_Width(other.m_Width),
        m_Height(other.m_Height),
        m_PitchBytes(other.m_PitchBytes),
        m_ElementSize(other.m_ElementSize)
    {
    }

    DeviceBuffer2D& DeviceBuffer2D::operator=(DeviceBuffer2D&& other) noexcept
    {
        if (this != &other)
        {
            Free();
            m_Data = std::exchange(other.m_Data, nullptr);
            m_Width = other.m_Width;
            m_Height = other.m_Height;
            m_PitchBytes = other.m_PitchBytes;
            m_ElementSize = other.m_ElementSize;
        }

        return *this;
    }

    std::expected<void, Core::Utils::Error> DeviceBuffer2D::Allocate(size_t width, size_t height, size_t elementSize)
    {
        auto freeResult = Free();
        if (!freeResult)
            return std::unexpected(freeResult.error());

        void* data = nullptr;
        size_t pitch = 0;

        cudaError_t error = cudaMallocPitch(&data, &pitch, width * elementSize, height);
        if (error != cudaSuccess)
            return std::unexpected(Utils::MakeCudaError("cudaMallocPitch", error));

        m_Data = data;
        m_Width = width;
        m_Height = height;
        m_PitchBytes = pitch;
        m_ElementSize = elementSize;
        return {};
    }

    std::expected<void, Core::Utils::Error> DeviceBuffer2D::UploadSync(const void* hostData, size_t hostPitchElement) const
    {
        assert(m_Data != nullptr);
        assert(hostData != nullptr);

        cudaError_t error = cudaMemcpy2D(
            m_Data,
            m_PitchBytes,
            hostData,
            hostPitchElement * m_ElementSize,
            m_Width * m_ElementSize,
            m_Height,
            cudaMemcpyHostToDevice);

        if (error != cudaSuccess)
            return std::unexpected(Utils::MakeCudaError("cudaMemcpy2D", error));

        return {};
    }

    std::expected<void, Core::Utils::Error> DeviceBuffer2D::UploadAsync(const void* hostData, size_t hostPitchElement, void* stream) const
    {
        assert(m_Data != nullptr);
        assert(hostData != nullptr);
        assert(stream != nullptr);

        cudaError_t error = cudaMemcpy2DAsync(
            m_Data,
            m_PitchBytes,
            hostData,
            hostPitchElement * m_ElementSize,
            m_Width * m_ElementSize,
            m_Height,
            cudaMemcpyHostToDevice,
            static_cast<cudaStream_t>(stream));

        if (error != cudaSuccess)
            return std::unexpected(Utils::MakeCudaError("cudaMemcpy2DAsync", error));

        return {};
    }

    std::expected<void, Core::Utils::Error> DeviceBuffer2D::MemsetBytesSync(uint8_t value) const
	{
		assert(m_Data != nullptr);

		cudaError_t error = cudaMemset2D(
			m_Data,
			m_PitchBytes,
			value,
			m_Width * m_ElementSize,
			m_Height);

		if (error != cudaSuccess)
			return std::unexpected(Utils::MakeCudaError("cudaMemset2D", error));

		return {};
	}

	std::expected<void, Core::Utils::Error> DeviceBuffer2D::MemsetBytesAsync(uint8_t value, void* stream) const
	{
		assert(m_Data != nullptr);
		assert(stream != nullptr);

		cudaError_t error = cudaMemset2DAsync(
			m_Data,
			m_PitchBytes,
			value,
			m_Width * m_ElementSize,
			m_Height,
			static_cast<cudaStream_t>(stream));

		if (error != cudaSuccess)
			return std::unexpected(Utils::MakeCudaError("cudaMemset2DAsync", error));

		return {};
	}

    std::expected<void, Core::Utils::Error> DeviceBuffer2D::Free()
    {
        if (m_Data == nullptr)
            return {};

        cudaError_t error = cudaFree(m_Data);
        if (error != cudaSuccess)
            return std::unexpected(Utils::MakeCudaError("cudaFree", error));

		m_Data = nullptr;
		m_Width = 0;
		m_Height = 0;
		m_PitchBytes = 0;
		m_ElementSize = 0;
        return {};
    }
}
