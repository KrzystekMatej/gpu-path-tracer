#include <Core/Graphics/Cuda/Runtime/Memory/DeviceBuffer2D.hpp>
#include <Core/Graphics/Cuda/Utils/Error.hpp>
#include <cuda_runtime.h>
#include <utility>

namespace Core::Graphics::Cuda::Runtime
{
    DeviceBuffer2D::~DeviceBuffer2D()
    {
        (void)Free();
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
            (void)Free();
            m_Data = std::exchange(other.m_Data, nullptr);
            m_Width = other.m_Width;
            m_Height = other.m_Height;
            m_PitchBytes = other.m_PitchBytes;
            m_ElementSize = other.m_ElementSize;
        }

        return *this;
    }

    std::expected<void, Core::Utils::Error> DeviceBuffer2D::Allocate(uint32_t width, uint32_t height, uint32_t elementSize)
    {
        CORE_TRY_DISCARD(Free());

        void* data = nullptr;
        size_t pitch = 0;

        CUDA_TRY("cudaMallocPitch", cudaMallocPitch(&data, &pitch, width * elementSize, height));

        m_Data = data;
        m_Width = width;
        m_Height = height;
        assert(pitch <= UINT32_MAX);
        m_PitchBytes = static_cast<uint32_t>(pitch);
        m_ElementSize = elementSize;
        return {};
    }

    std::expected<void, Core::Utils::Error> DeviceBuffer2D::Upload(const void* hostData, uint32_t hostPitchElement, const Stream& stream) const
    {
        assert(m_Data != nullptr);
        assert(hostData != nullptr);

        CUDA_TRY("cudaMemcpy2DAsync", cudaMemcpy2DAsync(
            m_Data,
            m_PitchBytes,
            hostData,
            hostPitchElement * m_ElementSize,
            m_Width * m_ElementSize,
            m_Height,
            cudaMemcpyKind::cudaMemcpyHostToDevice,
            stream.GetRawHandle()));

        return {};
    }

	std::expected<void, Core::Utils::Error> DeviceBuffer2D::MemsetBytes(uint8_t value, const Stream& stream) const
	{
		assert(m_Data != nullptr);

		CUDA_TRY("cudaMemset2DAsync", cudaMemset2DAsync(
			m_Data,
			m_PitchBytes,
			value,
			m_Width * m_ElementSize,
			m_Height,
			stream.GetRawHandle()));

		return {};
	}

    std::expected<void, Core::Utils::Error> DeviceBuffer2D::Free(const Stream& stream)
    {
        if (m_Data == nullptr)
            return {};

        CUDA_TRY("cudaFree", cudaFreeAsync(m_Data, stream.GetRawHandle()));

		m_Data = nullptr;
		m_Width = 0;
		m_Height = 0;
		m_PitchBytes = 0;
		m_ElementSize = 0;
        return {};
    }
}
