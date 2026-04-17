#include <expected>
#include <Core/Runtime/Application.hpp>
#include <App/SceneViewer/Ui/UiLayer.hpp>
#include <App/SceneViewer/Domain/AppModule.hpp>

int main()
{
	std::filesystem::path projectConfigPath = "../../../project-config.yaml";

	auto appModule = std::make_unique<App::SceneViewer::Domain::AppModule>();
	auto uiLayer = std::make_unique<App::SceneViewer::Ui::UiLayer>(appModule.get());

	auto appResult = Core::Runtime::Application::Create
	(
		std::move(appModule),
		std::move(uiLayer),
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
