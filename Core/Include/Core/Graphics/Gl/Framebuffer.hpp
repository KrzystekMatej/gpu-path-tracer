#pragma once
#include <cstdint>
#include <Core/Graphics/Gl/Renderbuffer.hpp>
#include <Core/Graphics/Gl/Resources/Texture.hpp>

namespace Core::Graphics::Gl
{
    class Framebuffer
    {
    public:
        Framebuffer();
        Framebuffer(const Framebuffer&) = delete;
        Framebuffer& operator=(const Framebuffer&) = delete;
        Framebuffer(Framebuffer&& other) noexcept;
        Framebuffer& operator=(Framebuffer&& other) noexcept;
        ~Framebuffer();

        void Bind() const;
        void AttachTexture(uint32_t attachment, const Texture& texture, int level = 0) const;
        void AttachRenderbuffer(uint32_t attachment, const Renderbuffer& rb) const;
        bool IsComplete() const;

        uint32_t GetId() const { return m_Id; }
    private:
        uint32_t m_Id = 0;
    };
}
