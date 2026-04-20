#include <expected>
#include <Core/Runtime/Application.hpp>
#include <App/LayerKeys.hpp>
#include <App/Scripts/Layer.hpp>
#include <App/CameraRecorder/Layer.hpp>
#include <App/PathTracer/Layer.hpp>
#include <App/Ui/Layer.hpp>

int main()
{
	std::filesystem::path projectConfigPath = "../../../project-config.yaml";

	auto appResult = Core::Runtime::Application::Create
	(
		Core::Window::Attributes::DefaultAttributes(),
		projectConfigPath
	);

	if (!appResult)
	{
		appResult.error().Log();
		return 1;
	}
	
	{
		std::unique_ptr<Core::Runtime::Application> app = std::move(appResult).value();
		app->PrintInfo();
		app->PushLayer<App::Scripts::Layer>(App::LayerKeys::Scripts);
		app->PushLayer<App::CameraRecorder::Layer>(App::LayerKeys::CameraRecorder);
		app->PushLayer<App::PathTracer::Layer>(App::LayerKeys::PathTracer);
		app->PushLayer<App::Ui::Layer>(App::LayerKeys::Ui);
		app->Run();
	}	

	return 0;
}
