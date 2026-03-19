#include "SceneViewerApp.hpp"
#include <GLFW/glfw3.h>
#include "Scripting/CameraController.hpp"

namespace App
{
	void SceneViewerApp::Init(const Core::App::InitContext& context)
	{
		context.scriptCatalog.Register("AwakeCameraController", App::Scripting::AwakeCameraController);
		context.scriptCatalog.Register("UpdateCameraController", App::Scripting::UpdateCameraController);

		context.resolverRegistry.RegisterResolver("CameraController", std::make_unique<App::Scripting::CameraControllerResolver>());

		m_Window = &context.window;
		context.eventDispatcher.sink<Core::Events::KeyPressed>().connect<&SceneViewerApp::OnKeyPressed>(*this);

		m_Window->SetCursorMode(Core::CursorMode::Disabled);
		m_Window->SetRawMouseMotionEnabled(true);
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