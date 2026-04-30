#include <glad/gl.h>
#include <Core/Graphics/Gl/Renderbuffer.hpp>
#include <cassert>
#include <utility>

namespace Core::Graphics::Gl
{
    Renderbuffer::Renderbuffer(uint32_t internalFormat)
        : m_Width(0), m_Height(0), m_InternalFormat(internalFormat)
    {
        glGenRenderbuffers(1, &m_Id);
    }

    Renderbuffer::Renderbuffer(uint32_t width, uint32_t height, uint32_t internalFormat)
        : m_Width(width), m_Height(height), m_InternalFormat(internalFormat)
    {
        glGenRenderbuffers(1, &m_Id);
        Allocate();
    }

    Renderbuffer::Renderbuffer(Renderbuffer&& other) noexcept
        : m_Id(std::exchange(other.m_Id, 0)),
        m_Width(other.m_Width),
        m_Height(other.m_Height),
        m_InternalFormat(other.m_InternalFormat)
    {
    }

	Renderbuffer& Renderbuffer::operator=(Renderbuffer&& other) noexcept
	{
		if (this != &other)
		{
			if (m_Id)
				glDeleteRenderbuffers(1, &m_Id);

			m_Id = std::exchange(other.m_Id, 0);
			m_Width = other.m_Width;
			m_Height = other.m_Height;
			m_InternalFormat = other.m_InternalFormat;
		}
		return *this;
	}

    Renderbuffer::~Renderbuffer()
    {
        if (m_Id)
        {
            glDeleteRenderbuffers(1, &m_Id);
            m_Id = 0;
        }
    }

    void Renderbuffer::Bind() const
    {
        glBindRenderbuffer(GL_RENDERBUFFER, m_Id);
    }

    void Renderbuffer::Allocate(uint32_t width, uint32_t height)
    {
        m_Width = width;
        m_Height = height;
        Allocate();
    }

    void Renderbuffer::Allocate()
    {
        assert(m_Id != 0);

        Bind();
        glRenderbufferStorage(GL_RENDERBUFFER, m_InternalFormat, m_Width, m_Height);
    }
}
