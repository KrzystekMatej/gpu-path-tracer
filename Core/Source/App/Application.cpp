#include <glad/gl.h>
#include "App/Application.hpp"
#include "App/Context.hpp"
#include "ECS/Systems/Render.hpp"
#include "IO/SceneLoader.hpp"
#include "ECS/Context.hpp"

namespace Core::App
{
	Application::Application(
		std::unique_ptr<Client> client, 
		Window window,
		Scripts::Catalog catalog,
		ECS::SceneResolverRegistry resolverRegistry,
		Assets::Manager assetManager, 
		Project project)
		: m_Client(std::move(client)), 
		m_Time(),
		m_Window(std::move(window)),
		m_Input(),
		m_EventDispatcher(),
		m_Renderer(m_Window.GetContext()),
		m_ScriptCatalog(std::move(catalog)),
		m_Scene(),
		m_ResolverRegistry(std::move(resolverRegistry)),
		m_ScriptRunner(),
		m_AssetManager(std::move(assetManager)), 
		m_Project(std::move(project))
	{ 
		m_Renderer.Initialize();
		m_Window.SetEventRouter(std::make_unique<Events::WindowEventRouter>(m_Input, m_EventDispatcher));
		m_Window.InitCallbacks();
		m_Client->RegisterEventHandlers(m_EventDispatcher, m_Window);
	}

	Application::~Application()
	{
		glfwTerminate();
	}

	std::expected<std::unique_ptr<Application>, Utils::Error> Application::Create
	(
		std::unique_ptr<Client> client, 
		WindowAttributes windowAttributes, 
		const std::filesystem::path& projectConfigPath
	)
	{
		if (!glfwInit())
			return std::unexpected(Utils::Error("Failed to initialize GLFW!"));

		auto windowResult = Window::Create(std::move(windowAttributes));
		if (!windowResult)
			return std::unexpected(std::move(windowResult).error());

		Window window = std::move(windowResult).value();

		window.GetContext().MakeCurrent();
		if (!gladLoadGL(glfwGetProcAddress))
			return std::unexpected(Utils::Error("Failed to initialize GLAD!"));
		window.GetContext().SetSwapInterval(1);

		Scripts::Catalog scriptCatalog;
		client->RegisterUserScripts(scriptCatalog);

		ECS::SceneResolverRegistry resolverRegistry;
		resolverRegistry.RegisterCoreResolvers();
		client->RegisterUserResolvers(resolverRegistry);

		auto projectResult = Project::Create(projectConfigPath);

		if (!projectResult)
			return std::unexpected(std::move(projectResult).error());

		Project project = std::move(projectResult).value();

		auto assetManagerResult = Assets::Manager::Create(project.GetContentPath());
		if (!assetManagerResult)
			return std::unexpected(std::move(assetManagerResult).error());

		std::unique_ptr<Application> app = std::unique_ptr<Application>(new Application(
			std::move(client), 
			std::move(window),
			std::move(scriptCatalog),
			std::move(resolverRegistry),
			std::move(assetManagerResult).value(), 
			std::move(project)));

		auto ok = app->SetScene(app->m_Project.GetStartScenePath());

		if (!ok)
			return std::unexpected(std::move(ok).error());

		return app;
	}

	std::expected<void, Utils::Error> Application::SetScene(const std::filesystem::path& path)
	{
		auto loadedScene = IO::LoadScene(path);
		if (!loadedScene)
			return std::unexpected(std::move(loadedScene).error());
		IO::Scene scene = std::move(loadedScene).value();
		auto resolvedScene = ECS::Scene::Create(std::move(scene), m_ResolverRegistry, m_AssetManager);
		if (!resolvedScene)
			return std::unexpected(std::move(resolvedScene).error());
		m_Scene = std::move(resolvedScene).value();

		auto ok = m_ScriptRunner.Bind(m_ScriptCatalog, m_Scene);
		if (!ok)
			return std::unexpected(std::move(ok).error());

		return {};
	}

	
	void Application::PrintInfo() const
	{
		GLVersion version = m_Window.GetContext().GetVersion();
		spdlog::info("OpenGL Version: {}.{}", version.Major, version.Minor);
		spdlog::info("Vendor: {}", version.Vendor);
		spdlog::info("Renderer: {}", version.Renderer);
		spdlog::info("GLSL: {}", version.GLSL);

		int major, minor, revision;
		glfwGetVersion(&major, &minor, &revision);
		spdlog::info("GLFW Version: {}.{}.{}", major, minor, revision);
	}

	void Application::Run()
	{
		Context appContext(m_Time, m_Window, m_Input, m_EventDispatcher, m_Project);
		ECS::Context sceneContext(m_Time, m_Window, m_Input, m_EventDispatcher, m_Scene);

		m_ScriptRunner.Awake(sceneContext);
		while (m_Window.IsOpen())
		{
			m_Time.Update();
			m_Input.BeginFrame();

			m_Window.PollEvents();
			m_EventDispatcher.update();

			m_Client->Update(appContext);

			m_ScriptRunner.Update(sceneContext);

			m_Renderer.BeginFrame();

			m_Renderer.Clear(0.1f, 0.1f, 0.1f, 1.0f);

			ECS::Systems::RenderScene(m_Renderer, m_Scene);

			m_Client->Render(appContext);

			m_Renderer.EndFrame();
			m_Window.SwapBuffers();
		}
	}
}
