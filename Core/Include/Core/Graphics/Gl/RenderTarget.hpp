#pragma once
#include <cstdint>
#include <Core/Graphics/Gl/Framebuffer.hpp>
#include <Core/Graphics/Gl/DynamicTexture2D.hpp>
#include <Core/Graphics/Gl/Renderbuffer.hpp>
#include <Core/Graphics/Gl/RenderSurface.hpp>

namespace Core::Graphics::Gl
{
    class RenderTarget
    {
    public:
        RenderTarget(uint32_t width, uint32_t height);
        RenderTarget(const RenderTarget&) = delete;
        RenderTarget& operator=(const RenderTarget&) = delete;
        RenderTarget(RenderTarget&&) noexcept = default;
        RenderTarget& operator=(RenderTarget&&) noexcept = default;

        void Bind() const;
        void Resize(uint32_t width, uint32_t height);
        bool IsComplete() const;


        uint32_t GetWidth() const { return m_ColorTexture.GetWidth(); }
        uint32_t GetHeight() const { return m_ColorTexture.GetHeight(); }

        const Resources::Texture& GetTexture() const { return m_ColorTexture.GetTexture(); }
        const Framebuffer& GetFramebuffer() const { return m_Framebuffer; }
		RenderSurface GetRenderSurface() const { return RenderSurface(m_Framebuffer.GetId(), GetWidth(), GetHeight()); }
    private:
        void Attach();

        Framebuffer m_Framebuffer;
        DynamicTexture2D m_ColorTexture;
        Renderbuffer m_DepthStencilBuffer;
    };
}
