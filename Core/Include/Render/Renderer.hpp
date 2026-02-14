#pragma once
#include "Window/GraphicsContext.hpp"

namespace Core
{
	class Renderer
    {
    public:
        Renderer(const GraphicsContext& context);
        void Initialize();
        void BeginFrame();
        void EndFrame();
        void Clear(float r, float g, float b, float a);

    private:
        const GraphicsContext& m_Context;
    };
}