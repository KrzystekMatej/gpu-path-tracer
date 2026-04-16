#pragma once
#include <cstdint>
#include <utility>
#include <vector>
#include <expected>
#include <Core/Import/Image.hpp>
#include <Core/Utils/Error.hpp>

namespace Core::Graphics::Cuda::Resources
{
    class Texture
    {
    public:
        ~Texture();
        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        static std::expected<Texture, Core::Utils::Error> Create2D(const Import::Image& image);
    private:
        Texture() = default;

        void* m_CudaArray = nullptr;
        uint64_t m_CudaTexture = 0;
    };
}
