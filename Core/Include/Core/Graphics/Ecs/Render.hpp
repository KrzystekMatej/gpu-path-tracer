#pragma once
#include <Core/Ecs/Scene.hpp>
#include <Core/Graphics/Gl/Renderer.hpp>

namespace Core::Graphics::Ecs
{
	void RenderScene(const Graphics::Gl::Renderer& renderer, const Core::Ecs::Scene& scene, const Assets::Storage& storage, float aspectRatio);
}