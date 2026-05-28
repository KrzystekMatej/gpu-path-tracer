#pragma once
#include <Core/Graphics/Gl/Resources/Texture.hpp>

namespace Core::Graphics::Gl
{
    struct TextureFormat
    {
        uint32_t internalFormat = 0;
        uint32_t externalFormat = 0;
        uint32_t pixelType = 0;
    };

    class Texture2D
    {
    public:
        Texture2D(TextureFormat format);
        Texture2D(uint32_t width, uint32_t height, TextureFormat format);
		Texture2D(const Texture2D&) = delete;
		Texture2D& operator=(const Texture2D&) = delete;
		Texture2D(Texture2D&&) noexcept = default;
		Texture2D& operator=(Texture2D&&) noexcept = default;
        
        static Texture2D CreateRgba8();

        void Allocate(uint32_t width, uint32_t height);
		void Upload(const void* data) const;

        const Texture& GetTexture() const { return m_Texture; }

        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
    private:
        void Init();
        void Allocate();

        Texture m_Texture;
        uint32_t m_Width;
        uint32_t m_Height;
        TextureFormat m_Format;
    };
}