#include "SceneViewerApp.hpp"
#include <GLFW/glfw3.h>
#include "Events/KeyEvent.hpp"
#include "Scripting/CameraController.hpp"

namespace App
{
	void SceneViewerApp::OnEvent(const Core::App::Context& context, const Core::Event& event)
	{
		if (event.GetType() == Core::EventType::Key)
		{
			Core::KeyEvent keyEvent = static_cast<const Core::KeyEvent&>(event);
			if (keyEvent.GetKey() == GLFW_KEY_ESCAPE && keyEvent.GetAction() == GLFW_PRESS)
			{
				context.window.Close();
			}
		}
	}
	void SceneViewerApp::Update(const Core::App::Context& context)
	{
	}
	void SceneViewerApp::Render(const Core::App::Context& context)
	{
	}

	void SceneViewerApp::RegisterUserScripts(Core::Scripts::Catalog& scriptCatalog)
	{
		scriptCatalog.Register("AwakeCameraController", App::Scripting::AwakeCameraController);
		scriptCatalog.Register("UpdateCameraController", App::Scripting::UpdateCameraController);
	}

	void SceneViewerApp::RegisterUserResolvers(Core::ECS::SceneResolverRegistry& resolverRegistry)
	{
		resolverRegistry.RegisterResolver("CameraController", std::make_unique<App::Scripting::CameraControllerResolver>());
	}
}