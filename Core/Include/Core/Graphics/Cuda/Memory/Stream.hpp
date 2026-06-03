#pragma once
#include <cuda_runtime.h>
#include <Core/Graphics/Cuda/Utils/Error.hpp>

namespace Core::Graphics::Cuda::Memory
{
    class Stream
    {
    public:
        Stream();
        ~Stream();

        Stream(const Stream&) = delete;
        Stream& operator=(const Stream&) = delete;

        Stream(Stream&& other) noexcept;
        Stream& operator=(Stream&& other) noexcept;
        
        std::expected<void, Core::Utils::Error> Synchronize() const;
        std::expected<bool, Core::Utils::Error> IsFinished() const;

        cudaStream_t GetRawHandle() const;
    private:
        cudaStream_t m_Handle = nullptr;
    };
}