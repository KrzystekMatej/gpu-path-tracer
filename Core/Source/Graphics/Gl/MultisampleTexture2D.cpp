#include <algorithm>
#include <cassert>
#include <glad/gl.h>
#include <Core/Graphics/Gl/MultisampleTexture2D.hpp>

namespace Core::Graphics::Gl
{
    MultisampleTexture2D::MultisampleTexture2D(uint32_t internalFormat)
        : m_Texture(GL_TEXTURE_2D_MULTISAMPLE),
          m_InternalFormat(internalFormat)
    {
    }

    MultisampleTexture2D::MultisampleTexture2D(
        uint32_t width,
        uint32_t height,
        uint32_t internalFormat,
        uint32_t samples)
        : m_Texture(GL_TEXTURE_2D_MULTISAMPLE),
          m_Width(width),
          m_Height(height),
          m_InternalFormat(internalFormat),
          m_Samples(std::max(samples, 1u))
    {
        Allocate();
    }

    void MultisampleTexture2D::Allocate(uint32_t width, uint32_t height)
    {
        m_Width = width;
        m_Height = height;
        Allocate();
    }

    void MultisampleTexture2D::Allocate(uint32_t width, uint32_t height, uint32_t samples)
    {
        m_Width = width;
        m_Height = height;
        m_Samples = std::max(samples, 1u);
        Allocate();
    }

    void MultisampleTexture2D::Allocate()
    {
        assert(m_Texture.GetId() != 0);

        m_Texture.Bind();

        glTexImage2DMultisample(
            m_Texture.GetTarget(),
            static_cast<GLsizei>(m_Samples),
            m_InternalFormat,
            static_cast<GLsizei>(m_Width),
            static_cast<GLsizei>(m_Height),
            GL_TRUE);
    }
}