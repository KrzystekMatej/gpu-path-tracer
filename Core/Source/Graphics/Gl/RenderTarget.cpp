#include <cassert>
#include <glad/gl.h>
#include <Core/Graphics/Gl/RenderTarget.hpp>

namespace Core::Graphics::Gl
{
    RenderTarget::RenderTarget(uint32_t width, uint32_t height)
        : m_ColorTexture(
            width,
            height,
            TextureFormat
            {
                GL_RGBA8,
                GL_RGBA,
                GL_UNSIGNED_BYTE
            }),
          m_DepthStencilBuffer(width, height, GL_DEPTH24_STENCIL8)
    {
        Attach();
        assert(m_ColorTexture.GetWidth() == m_DepthStencilBuffer.GetWidth());
        assert(m_ColorTexture.GetHeight() == m_DepthStencilBuffer.GetHeight());
        assert(IsComplete());
    }

    void RenderTarget::Bind() const
    {
        m_Framebuffer.Bind();
    }

    void RenderTarget::Resize(uint32_t width, uint32_t height)
    {
        m_ColorTexture.Allocate(width, height);
        m_DepthStencilBuffer.Allocate(width, height);

        Attach();
        assert(m_ColorTexture.GetWidth() == m_DepthStencilBuffer.GetWidth());
        assert(m_ColorTexture.GetHeight() == m_DepthStencilBuffer.GetHeight());
        assert(IsComplete());
    }

    bool RenderTarget::IsComplete() const
    {
        m_Framebuffer.Bind();
        return m_Framebuffer.IsComplete();
    }

    void RenderTarget::Attach()
    {
        m_Framebuffer.Bind();
        m_Framebuffer.AttachTexture(GL_COLOR_ATTACHMENT0, m_ColorTexture.GetTexture());
        m_Framebuffer.AttachRenderbuffer(GL_DEPTH_STENCIL_ATTACHMENT, m_DepthStencilBuffer);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
    }
}