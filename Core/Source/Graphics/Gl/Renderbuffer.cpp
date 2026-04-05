#include <glad/glad.h>
#include "Graphics/Gl/Renderbuffer.hpp"
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
        m_Width(std::exchange(other.m_Width, 0)),
        m_Height(std::exchange(other.m_Height, 0)),
        m_InternalFormat(std::exchange(other.m_InternalFormat, 0)) { }

	Renderbuffer& Renderbuffer::operator=(Renderbuffer&& other) noexcept
	{
		if (this != &other)
		{
			if (m_Id)
				glDeleteRenderbuffers(1, &m_Id);

			m_Id = std::exchange(other.m_Id, 0);
			m_Width = std::exchange(other.m_Width, 0);
			m_Height = std::exchange(other.m_Height, 0);
			m_InternalFormat = std::exchange(other.m_InternalFormat, 0);
		}
		return *this;
	}

    Renderbuffer::~Renderbuffer()
    {
        if (m_Id) glDeleteRenderbuffers(1, &m_Id);
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
        assert(m_Width != 0);
        assert(m_Height != 0);
        assert(m_InternalFormat != 0);

        Bind();
        glRenderbufferStorage(GL_RENDERBUFFER, m_InternalFormat, m_Width, m_Height);
    }
}
