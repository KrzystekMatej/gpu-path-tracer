#pragma once
#include <cstdint>
#include <Core/Graphics/Gl/Resources/Texture.hpp>

namespace Core::Graphics::Gl
{
    class MultisampleTexture2D
    {
    public:
        MultisampleTexture2D(uint32_t internalFormat);
        MultisampleTexture2D(uint32_t width, uint32_t height, uint32_t internalFormat, uint32_t samples);

        MultisampleTexture2D(const MultisampleTexture2D&) = delete;
        MultisampleTexture2D& operator=(const MultisampleTexture2D&) = delete;
        MultisampleTexture2D(MultisampleTexture2D&&) noexcept = default;
        MultisampleTexture2D& operator=(MultisampleTexture2D&&) noexcept = default;

        void Allocate(uint32_t width, uint32_t height);
        void Allocate(uint32_t width, uint32_t height, uint32_t samples);

        const Texture& GetTexture() const { return m_Texture; }

        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
        uint32_t GetSamples() const { return m_Samples; }
        uint32_t GetInternalFormat() const { return m_InternalFormat; }

    private:
        void Allocate();

        Texture m_Texture;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        uint32_t m_InternalFormat = 0;
        uint32_t m_Samples = 1;
    };
}