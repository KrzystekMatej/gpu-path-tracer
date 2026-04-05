#include <glad/gl.h>
#include "Graphics/Gl/DynamicTexture2D.hpp"

namespace Core::Graphics::Gl
{
    DynamicTexture2D::DynamicTexture2D(TextureFormat format)
        : m_Texture(GL_TEXTURE_2D), m_Width(0), m_Height(0), m_Format(format)
    {
        Init();
    }

    DynamicTexture2D::DynamicTexture2D(uint32_t width, uint32_t height, TextureFormat format)
        : m_Texture(GL_TEXTURE_2D), m_Width(width), m_Height(height), m_Format(format)
    {
        Init();
        Allocate();
    }

    void DynamicTexture2D::Allocate(uint32_t width, uint32_t height)
	{
        m_Width = width;
        m_Height = height;
        Allocate();
	}

    void DynamicTexture2D::Init()
    {
        m_Texture.Bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    void DynamicTexture2D::Allocate()
	{
        assert(m_Width != 0);
        assert(m_Height != 0);
        assert(m_Format.externalFormat != 0);

		m_Texture.Bind();
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			static_cast<GLint>(m_Format.internalFormat),
			static_cast<GLsizei>(m_Width),
			static_cast<GLsizei>(m_Height),
			0,
			m_Format.externalFormat,
			m_Format.pixelType,
			nullptr);
	}
}