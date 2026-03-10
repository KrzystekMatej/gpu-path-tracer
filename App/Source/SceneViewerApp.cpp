#include "SceneViewerApp.hpp"
#include <GLFW/glfw3.h>
#include "Scripting/CameraController.hpp"

namespace App
{
	void SceneViewerApp::RegisterUserScripts(Core::Scripts::Catalog& scriptCatalog) const
	{
		scriptCatalog.Register("AwakeCameraController", App::Scripting::AwakeCameraController);
		scriptCatalog.Register("UpdateCameraController", App::Scripting::UpdateCameraController);
	}

	void SceneViewerApp::RegisterUserResolvers(Core::ECS::SceneResolverRegistry& resolverRegistry) const
	{
		resolverRegistry.RegisterResolver("CameraController", std::make_unique<App::Scripting::CameraControllerResolver>());
	}

	void SceneViewerApp::RegisterEventHandlers(entt::dispatcher& dispatcher, Core::Window& window)
	{
		m_Window = &window;
		dispatcher.sink<Core::Events::KeyPressed>().connect<&SceneViewerApp::OnKeyPressed>(*this);
	}

	void SceneViewerApp::OnKeyPressed(const Core::Events::KeyPressed& event)
	{
		if (!event.repeat && event.key == Core::Input::KeyCode::Escape)
		{
			m_Window->Close();
		}
	}

	void SceneViewerApp::Update(const Core::App::Context& context)
	{
	}
	void SceneViewerApp::Render(const Core::App::Context& context)
	{
	}
}