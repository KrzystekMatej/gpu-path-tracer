#pragma once
#include <cstdint>

namespace Core::Graphics::Gl
{
    class Renderbuffer
    {
    public:
        Renderbuffer(uint32_t internalFormat);
        Renderbuffer(uint32_t width, uint32_t height, uint32_t internalFormat);
        Renderbuffer(const Renderbuffer&) = delete;
        Renderbuffer& operator=(const Renderbuffer&) = delete;
        Renderbuffer(Renderbuffer&& other) noexcept;
        Renderbuffer& operator=(Renderbuffer&& other) noexcept;
        ~Renderbuffer();

        void Bind() const;
        void Allocate(uint32_t width, uint32_t height);
        uint32_t GetId() const { return m_Id; }
        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
    private:
        void Allocate();

        uint32_t m_Id = 0;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        uint32_t m_InternalFormat = 0;
    };
}
