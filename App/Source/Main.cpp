#include "App/Application.hpp"
#include "SceneViewerApp.hpp"
#include "Project/ProjectConfig.hpp"
#include <expected>


int main()
{
	std::filesystem::path projectConfigPath = "../../../project-config.yaml";

	auto appResult = Core::App::Application::Create
	(
		std::make_unique<App::SceneViewerApp>(),
		Core::WindowAttributes::DefaultAttributes(),
		projectConfigPath
	);

	if (!appResult)
	{
		appResult.error().Log();
		return 1;
	}
	
	
	{
		std::unique_ptr<Core::App::Application> app = std::move(appResult).value();
		app->PrintInfo();
		app->Run();
	}	

	Core::Window::TerminateBackend();
	return 0;
}
