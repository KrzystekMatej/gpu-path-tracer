#pragma once
#include <expected>
#include <cuda_runtime.h>
#include <Core/Graphics/Cuda/Utils/Error.hpp>

namespace Core::Graphics::Cuda::Runtime
{
    class Stream
    {
    public:
        Stream() = default;
        static const Stream& Default();
        static std::expected<Stream, Core::Utils::Error> Create();
        ~Stream();

        Stream(const Stream&) = delete;
        Stream& operator=(const Stream&) = delete;

        Stream(Stream&& other) noexcept;
        Stream& operator=(Stream&& other) noexcept;
        
        std::expected<void, Core::Utils::Error> Synchronize() const;
        std::expected<bool, Core::Utils::Error> IsFinished() const;

        bool IsDefault() const { return m_Handle == nullptr; }
        bool IsOwning() const { return m_Handle != nullptr; }

        cudaStream_t GetRawHandle() const;
    private:
        cudaStream_t m_Handle = nullptr;
    };
}