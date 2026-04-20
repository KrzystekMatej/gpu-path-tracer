#include <App/Scripts/Layer.hpp>
#include <App/Scripts/CameraController.hpp>
#include <Core/Runtime/Application.hpp>

namespace App::Scripts
{
	void Layer::OnAttach()
	{
		auto& scriptCatalog = Core::Runtime::Application::ScriptCatalog();
		auto& builderRegistry = Core::Runtime::Application::BuilderRegistry();

		scriptCatalog.Register("AwakeCameraController", Scripts::AwakeCameraController);
		scriptCatalog.Register("UpdateCameraController", Scripts::UpdateCameraController);
		builderRegistry.Register("CameraController", std::make_unique<Scripts::CameraControllerBuilder>());
	}
}