#include <algorithm>
#include <cassert>
#include <glad/gl.h>
#include <Core/Graphics/Gl/RenderTarget.hpp>

namespace Core::Graphics::Gl
{
    uint32_t RenderTarget::ClampDimension(uint32_t value)
    {
        return std::max(value, 1u);
    }

    uint32_t RenderTarget::ClampSamples(uint32_t samples)
    {
        GLint maxSamples = 1;
        glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);

        const uint32_t safeMaxSamples = static_cast<uint32_t>(std::max(maxSamples, 1));
        return std::clamp(samples, 1u, safeMaxSamples);
    }

    RenderTarget::RenderTarget(uint32_t samples)
        : m_Samples(ClampSamples(samples)),
          m_ColorTexture(GL_RGBA8),
          m_DepthStencilBuffer(GL_DEPTH24_STENCIL8),
          m_ResolvedColorTexture(Texture2D::CreateRgba8())
    {
        Attach();
    }

    void RenderTarget::Bind() const
    {
        m_Framebuffer.Bind();
    }

    void RenderTarget::Resize(uint32_t width, uint32_t height)
    {
        Allocate(width, height);
        Attach();

        assert(IsComplete());
    }

    void RenderTarget::Resolve() const
    {
        GLint previousReadFramebuffer = 0;
        GLint previousDrawFramebuffer = 0;

        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &previousReadFramebuffer);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previousDrawFramebuffer);

        m_Framebuffer.BindRead();
        m_ResolveFramebuffer.BindDraw();

        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);

        glBlitFramebuffer(
            0,
            0,
            static_cast<GLint>(GetWidth()),
            static_cast<GLint>(GetHeight()),
            0,
            0,
            static_cast<GLint>(GetWidth()),
            static_cast<GLint>(GetHeight()),
            GL_COLOR_BUFFER_BIT,
            GL_NEAREST);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, previousReadFramebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, previousDrawFramebuffer);
    }

    bool RenderTarget::IsComplete() const
    {
        m_Framebuffer.Bind();
        const bool framebufferComplete = m_Framebuffer.IsComplete();

        m_ResolveFramebuffer.Bind();
        const bool resolveFramebufferComplete = m_ResolveFramebuffer.IsComplete();

        return framebufferComplete && resolveFramebufferComplete;
    }

    void RenderTarget::Allocate(uint32_t width, uint32_t height)
    {
        const uint32_t safeWidth = ClampDimension(width);
        const uint32_t safeHeight = ClampDimension(height);

        m_ColorTexture.Allocate(safeWidth, safeHeight, m_Samples);
        m_DepthStencilBuffer.Allocate(safeWidth, safeHeight, m_Samples);
        m_ResolvedColorTexture.Allocate(safeWidth, safeHeight);

        assert(m_ColorTexture.GetWidth() == m_DepthStencilBuffer.GetWidth());
        assert(m_ColorTexture.GetHeight() == m_DepthStencilBuffer.GetHeight());
        assert(m_ColorTexture.GetWidth() == m_ResolvedColorTexture.GetWidth());
        assert(m_ColorTexture.GetHeight() == m_ResolvedColorTexture.GetHeight());
    }

    void RenderTarget::Attach()
    {
        m_Framebuffer.Bind();
        m_Framebuffer.AttachTexture(GL_COLOR_ATTACHMENT0, m_ColorTexture.GetTexture());
        m_Framebuffer.AttachRenderbuffer(GL_DEPTH_STENCIL_ATTACHMENT, m_DepthStencilBuffer);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);

        m_ResolveFramebuffer.Bind();
        m_ResolveFramebuffer.AttachTexture(GL_COLOR_ATTACHMENT0, m_ResolvedColorTexture.GetTexture());
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
    }
}