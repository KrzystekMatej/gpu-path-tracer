#include <Core/Graphics/Cuda/Memory/SharedCounter.hpp>
#include <Core/Graphics/Cuda/Utils/Error.hpp>
#include <cuda_runtime.h>
#include <utility>

namespace Core::Graphics::Cuda::Memory
{
    SharedCounter::~SharedCounter()
    {
        Free();
    }

    SharedCounter::SharedCounter(SharedCounter&& other) noexcept
        : m_DeviceBuffer(std::move(other.m_DeviceBuffer))
        , m_HostValue(std::exchange(other.m_HostValue, 0))
    {
    }

    SharedCounter& SharedCounter::operator=(SharedCounter&& other) noexcept
    {
        if (this != &other)
        {
            Free();
            m_DeviceBuffer = std::move(other.m_DeviceBuffer);
            m_HostValue = std::exchange(other.m_HostValue, 0);
        }

        return *this;
    }

    std::expected<void, Core::Utils::Error> SharedCounter::Allocate()
    {
        auto freeResult = Free();
        if (!freeResult)
            return std::unexpected(freeResult.error());

        m_HostValue = 0;

        auto allocateResult = m_DeviceBuffer.Allocate(sizeof(uint32_t), sizeof(uint32_t));
        if (!allocateResult)
            return std::unexpected(allocateResult.error());

        return Reset();
    }

    std::expected<void, Core::Utils::Error> SharedCounter::Free()
    {
        m_HostValue = 0;
        return m_DeviceBuffer.Free();
    }

    std::expected<void, Core::Utils::Error> SharedCounter::Reset()
    {
        m_HostValue = 0;
        return m_DeviceBuffer.MemsetBytesSync(0);
    }

    std::expected<void, Core::Utils::Error> SharedCounter::SyncFromDevice()
    {
        assert(m_DeviceBuffer.GetData() != nullptr);


        cudaError_t error = cudaMemcpy(
            &m_HostValue,
            m_DeviceBuffer.GetData(),
            sizeof(uint32_t),
            cudaMemcpyDeviceToHost);

        if (error != cudaSuccess)
            return std::unexpected(Utils::MakeCudaError("cudaMemcpy", error));

        return {};
    }

    std::expected<void, Core::Utils::Error> SharedCounter::SyncFromHost()
    {
        assert(m_DeviceBuffer.GetData() != nullptr);
		return m_DeviceBuffer.MemsetBytesSync(m_HostValue);
    }

    uint32_t* SharedCounter::GetDevicePointer() const
    {
        return reinterpret_cast<uint32_t*>(m_DeviceBuffer.GetData());
    }
}