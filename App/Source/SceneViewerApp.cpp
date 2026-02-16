#include "SceneViewerApp.hpp"
#include <GLFW/glfw3.h>
#include "Events/KeyEvent.hpp"

namespace App
{
	void SceneViewerApp::OnEvent(const Core::AppContext& context, const Core::Event& event)
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
	void SceneViewerApp::Update(const Core::AppContext& context)
	{
		// Update scene logic, animations, etc. here
	}
	void SceneViewerApp::Render(const Core::AppContext& context)
	{
		// Render the scene here
	}
}