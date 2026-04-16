#include <glad/glad.h>
#include <Core/Graphics/Gl/Framebuffer.hpp>
#include <Core/Graphics/Gl/Renderbuffer.hpp>
#include <Core/Graphics/Gl/Resources/Texture.hpp>

namespace Core::Graphics::Gl
{
    Framebuffer::Framebuffer()
    {
        glGenFramebuffers(1, &m_Id);
    }

    Framebuffer::~Framebuffer()
    {
        if (m_Id)
        {
            glDeleteFramebuffers(1, &m_Id);
            m_Id = 0;
        }
    }

    Framebuffer::Framebuffer(Framebuffer&& other) noexcept
		: m_Id(std::exchange(other.m_Id, 0)) {}

    Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
	{
		if (this != &other)
		{
			if (m_Id)
				glDeleteFramebuffers(1, &m_Id);

			m_Id = std::exchange(other.m_Id, 0);
		}
		return *this;
	}

    void Framebuffer::Bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_Id);
    }

    void Framebuffer::AttachTexture(uint32_t attachment, const Resources::Texture& texture, int level) const
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, texture.GetTarget(), texture.GetId(), level);
    }

    void Framebuffer::AttachRenderbuffer(uint32_t attachment, const Renderbuffer& rb) const
    {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rb.GetId());
    }

    bool Framebuffer::IsComplete() const
    {
        return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    }
}
