#include <glad/gl.h>
#include "App/Application.hpp"
#include "App/Context.hpp"
#include "ECS/Systems/Render.hpp"
#include "IO/SceneLoader.hpp"
#include "ECS/Context.hpp"
#include "ECS/Systems/Transform.hpp"

namespace Core::App
{
	Application::Application(
		std::unique_ptr<Client> client, 
		Window window,
		Graphics::Gl::Renderer renderer,
		Scripts::Catalog catalog,
		ECS::SceneResolverRegistry resolverRegistry,
		Assets::Manager assetManager, 
		Project project)
		: m_Client(std::move(client)), 
		m_Time(),
		m_Window(std::move(window)),
		m_Input(),
		m_EventDispatcher(),
		m_Renderer(std::move(renderer)),
		m_ScriptCatalog(std::move(catalog)),
		m_Scene(),
		m_ResolverRegistry(std::move(resolverRegistry)),
		m_ScriptRunner(),
		m_AssetManager(std::move(assetManager)), 
		m_Project(std::move(project))
	{ 
		m_Window.SetEventRouter(std::make_unique<Events::WindowEventRouter>(m_Input, m_EventDispatcher));
		m_Window.InitCallbacks();
		m_Renderer.InitContext(&m_Window.GetContext());
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

		ECS::SceneResolverRegistry resolverRegistry;
		resolverRegistry.RegisterCoreResolvers();

		auto projectResult = Project::Create(projectConfigPath);

		if (!projectResult)
			return std::unexpected(std::move(projectResult).error());

		Project project = std::move(projectResult).value();

		auto assetManagerResult = Assets::Manager::Create(project.GetContentPath());
		if (!assetManagerResult)
			return std::unexpected(std::move(assetManagerResult).error());

		Assets::Manager assetManager = std::move(assetManagerResult).value();

		auto rendererResult = Graphics::Gl::Renderer::Create(assetManager);
		if (!rendererResult)
			return std::unexpected(std::move(rendererResult).error());

		std::unique_ptr<Application> app = std::unique_ptr<Application>(new Application(
			std::move(client),
			std::move(window),
			std::move(std::move(rendererResult).value()),
			std::move(scriptCatalog),
			std::move(resolverRegistry),
			std::move(assetManager), 
			std::move(project)));

		InitContext context(
			app->m_Window, 
			app->m_EventDispatcher, 
			app->m_ScriptCatalog,
			app->m_ResolverRegistry, 
			app->m_Project);

		app->m_Client->Init(context);
		
		auto initSceneResult = app->SetScene(app->m_Project.GetStartScenePath());

		if (!initSceneResult)
			return std::unexpected(std::move(initSceneResult).error());

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
			ECS::Systems::UpdateWorldTransforms(m_Scene);

			m_Renderer.BeginFrame();

			m_Renderer.SetViewport(0, 0, m_Window.GetWidth(), m_Window.GetHeight());
			m_Renderer.Clear(0.1f, 0.1f, 0.1f, 1.0f);

			ECS::Systems::RenderScene(m_Window.GetWidth() / (float)m_Window.GetHeight(), m_Renderer, m_Scene, m_AssetManager.GetStorage());

			m_Client->Render(appContext);

			m_Renderer.EndFrame();
			m_Window.SwapBuffers();
		}
	}
}
