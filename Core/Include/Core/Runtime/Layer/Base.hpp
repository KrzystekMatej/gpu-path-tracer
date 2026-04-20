#pragma once
#include <optional>
#include <Core/Graphics/Services/SceneRenderer.hpp>

namespace Core::Runtime::Layer
{
	class Base
	{
	public:
		Base() = default;
		virtual ~Base() = default;
		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnBuildUi() {}
		virtual void OnRender(Graphics::Services::SceneRenderer renderer) {}
	};
}