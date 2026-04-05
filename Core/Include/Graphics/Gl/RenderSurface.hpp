#pragma once
#include <cstdint>
#include "Graphics/Gl/Framebuffer.hpp"

namespace Core::Graphics::Gl
{
    class RenderSurface
    {
    public:
		RenderSurface() = default;
        RenderSurface(uint32_t framebufferId, uint32_t width, uint32_t height)
            : m_FramebufferId(framebufferId), m_Width(width), m_Height(height) {}

		uint32_t GetFramebufferId() const { return m_FramebufferId; }
        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
    private:
		uint32_t m_FramebufferId = 0;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
    };
}
