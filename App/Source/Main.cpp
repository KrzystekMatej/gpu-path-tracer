#include "App/Application.hpp"
#include "SceneViewerApp.hpp"
#include "Project/ProjectConfig.hpp"
#include <expected>


int main()
{
	App::SceneViewerApp appClient;
	std::filesystem::path projectConfigPath = "../../../project-config.yaml";

	auto result = Core::Application::Create
	(
		std::make_unique<App::SceneViewerApp>(appClient),
		Core::WindowAttributes::DefaultAttributes(),
		projectConfigPath
	);

	if (!result)
	{
		result.error().Log();
		return 1;
	}
	
	
	Core::Application app = std::move(result.value());
	app.PrintInfo();
	app.Run();
	return 0;
}
