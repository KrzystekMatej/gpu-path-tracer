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

    class DynamicTexture2D
    {
    public:
        DynamicTexture2D(TextureFormat format);
        DynamicTexture2D(uint32_t width, uint32_t height, TextureFormat format);
		DynamicTexture2D(const DynamicTexture2D&) = delete;
		DynamicTexture2D& operator=(const DynamicTexture2D&) = delete;
		DynamicTexture2D(DynamicTexture2D&&) noexcept = default;
		DynamicTexture2D& operator=(DynamicTexture2D&&) noexcept = default;

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