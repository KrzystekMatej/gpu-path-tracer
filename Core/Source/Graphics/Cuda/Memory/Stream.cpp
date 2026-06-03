#include <Core/Graphics/Cuda/Memory/Stream.hpp>
#include <utility>

namespace Core::Graphics::Cuda::Memory
{
    Stream::Stream()
    {
        cudaStreamCreate(&m_Handle);
    }

    Stream::~Stream()
    {
        cudaStreamDestroy(m_Handle);
    }

    Stream::Stream(Stream&& other) noexcept
        : m_Handle(std::exchange(other.m_Handle, nullptr))
    {
    }

    Stream& Stream::operator=(Stream&& other) noexcept
    {
        if (this != &other)
        {
            if (m_Handle)
                cudaStreamDestroy(m_Handle);

            m_Handle = std::exchange(other.m_Handle, nullptr);
        }
        return *this;
    }

    std::expected<void, Core::Utils::Error> Stream::Synchronize() const
    {
        cudaError_t result = cudaStreamSynchronize(m_Handle);
        if (result != cudaSuccess)
            return std::unexpected(Utils::MakeCudaError("cudaStreamSynchronize", result));
        return {};
    }

    std::expected<bool, Core::Utils::Error> Stream::IsFinished() const
    {
        cudaError_t result = cudaStreamQuery(m_Handle);
        if (result == cudaSuccess)
            return true;
        if (result == cudaErrorNotReady)
            return false;
        
        return std::unexpected(Utils::MakeCudaError("cudaStreamQuery", result));
    }

    cudaStream_t Stream::GetRawHandle() const
    {
        return m_Handle;
    }
}