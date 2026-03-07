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
	
	
	std::unique_ptr<Core::App::Application> app = std::move(appResult.value());
	app->PrintInfo();
	auto ok = app->SetScene("cornell-box.yaml");
	if (!ok)
		ok.error().Log();
	app->Run();
	return 0;
}
