#include <expected>
#include <Core/Runtime/Application.hpp>
#include <App/SceneViewerApp.hpp>


int main()
{
	std::filesystem::path projectConfigPath = "../../../project-config.yaml";

	auto appResult = Core::Runtime::Application::Create
	(
		std::make_unique<App::SceneViewerApp>(),
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
		app->Run();
	}	

	return 0;
}
