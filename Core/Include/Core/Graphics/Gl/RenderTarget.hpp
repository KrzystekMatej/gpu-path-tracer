#pragma once
#include <cstdint>
#include <Core/Graphics/Gl/Framebuffer.hpp>
#include <Core/Graphics/Gl/MultisampleTexture2D.hpp>
#include <Core/Graphics/Gl/Renderbuffer.hpp>
#include <Core/Graphics/Gl/RenderSurface.hpp>
#include <Core/Graphics/Gl/Texture2D.hpp>

namespace Core::Graphics::Gl
{
    class RenderTarget
    {
    public:
        RenderTarget(uint32_t samples);

        RenderTarget(const RenderTarget&) = delete;
        RenderTarget& operator=(const RenderTarget&) = delete;
        RenderTarget(RenderTarget&&) noexcept = default;
        RenderTarget& operator=(RenderTarget&&) noexcept = default;

        void Bind() const;
        void Resize(uint32_t width, uint32_t height);
        void Resolve() const;

        bool IsComplete() const;

        uint32_t GetWidth() const { return m_ResolvedColorTexture.GetWidth(); }
        uint32_t GetHeight() const { return m_ResolvedColorTexture.GetHeight(); }
        uint32_t GetSamples() const { return m_Samples; }

        const Texture& GetTexture() const { return m_ResolvedColorTexture.GetTexture(); }
        const Framebuffer& GetFramebuffer() const { return m_Framebuffer; }

        RenderSurface GetRenderSurface() const
        {
            return RenderSurface(m_Framebuffer.GetId(), GetWidth(), GetHeight());
        }

    private:
        static uint32_t ClampDimension(uint32_t value);
        static uint32_t ClampSamples(uint32_t samples);

        void Allocate(uint32_t width, uint32_t height);
        void Attach();

        uint32_t m_Samples = 1;

        Framebuffer m_Framebuffer;
        MultisampleTexture2D m_ColorTexture;
        Renderbuffer m_DepthStencilBuffer;

        Framebuffer m_ResolveFramebuffer;
        Texture2D m_ResolvedColorTexture;
    };
}