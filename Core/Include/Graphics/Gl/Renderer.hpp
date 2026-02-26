#pragma once
#include "Window/GraphicsContext.hpp"

namespace Core::Graphics::Gl
{
	class Renderer
    {
    public:
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = default;
		Renderer& operator=(Renderer&&) noexcept = default;

        Renderer(const GraphicsContext& context);
        void Initialize();
        void BeginFrame();
        void EndFrame();
        void Clear(float r, float g, float b, float a);

    private:
        const GraphicsContext& m_Context;
    };
}