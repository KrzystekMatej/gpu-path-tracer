#include <Core/Graphics/Cuda/Runtime/Stream.hpp>
#include <utility>

namespace Core::Graphics::Cuda::Runtime
{
    const Stream& Stream::Default()
    {
        static const Stream stream;
        return stream;
    }

    std::expected<Stream, Core::Utils::Error> Stream::Create()
    {
        Stream stream;
        CUDA_TRY("cudaStreamCreate", cudaStreamCreate(&stream.m_Handle));
        return stream;
    }

    Stream::~Stream()
    {
        if (m_Handle)
        {
            cudaStreamDestroy(m_Handle);
            m_Handle = nullptr;
        }
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
        CUDA_TRY("cudaStreamSynchronize", cudaStreamSynchronize(m_Handle));
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