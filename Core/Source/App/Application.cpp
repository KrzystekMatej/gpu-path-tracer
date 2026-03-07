#include <glad/gl.h>
#include "App/Application.hpp"
#include "App/Context.hpp"
#include "ECS/Systems/Render.hpp"
#include "IO/SceneLoader.hpp"

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
		m_Renderer(m_Window.GetContext()),
		m_ScriptCatalog(std::move(catalog)),
		m_Scene(),
		m_ResolverRegistry(std::move(resolverRegistry)),
		m_ScriptRunner(),
		m_AssetManager(std::move(assetManager)), 
		m_Project(std::move(project))
	{ 
		m_Renderer.Initialize();
		m_Window.InitCallbacks();
		m_Window.SetEventCallback([this](const Event& event) { OnEvent(event); });
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
			return std::unexpected(windowResult.error());

		Window window = std::move(windowResult.value());

		window.GetContext().MakeCurrent();
		if (!gladLoadGL(glfwGetProcAddress))
			return std::unexpected(Utils::Error("Failed to initialize GLAD!"));
		window.GetContext().SetSwapInterval(1);

		Scripts::Catalog scriptCatalog;
		client->RegisterScripts(scriptCatalog);

		ECS::SceneResolverRegistry resolverRegistry;
		client->RegisterSceneResolvers(resolverRegistry);

		auto projectResult = Project::Create(projectConfigPath);

		if (!projectResult)
			return std::unexpected(projectResult.error());

		Project project = std::move(projectResult.value());

		auto assetManagerResult = Assets::Manager::Create(project.GetAssetsPath());
		if (!assetManagerResult)
			return std::unexpected(assetManagerResult.error());

		return std::unique_ptr<Application>(new Application(
			std::move(client), 
			std::move(window),
			std::move(scriptCatalog),
			std::move(resolverRegistry),
			std::move(assetManagerResult.value()), 
			std::move(project)));
	}

	std::expected<void, Utils::Error> Application::SetScene(const std::filesystem::path& path)
	{
		auto loadedScene = IO::LoadScene(m_Project.GetScenesPath() / path);

		if (!loadedScene)
			return std::unexpected(loadedScene.error());

		auto resolvedScene = ECS::Scene::Create(std::move(loadedScene.value()), m_ResolverRegistry);

		auto ok = m_ScriptRunner.Bind(m_ScriptCatalog, resolvedScene.value());
		if (!ok)
			return std::unexpected(ok.error());

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


	void Application::OnEvent(const Event& event)
	{
		m_Client->OnEvent(Context(m_Time, m_Window, m_Project), event);
	}

	void Application::Run()
	{
		m_ScriptRunner.Awake(m_Scene);

		while (m_Window.IsOpen())
		{
			m_Time.Update();
			m_Window.PollEvents();

			Context context(m_Time, m_Window, m_Project);

			m_Client->Update(context);

			m_ScriptRunner.Update(m_Scene, m_Time);

			m_Renderer.BeginFrame();

			m_Renderer.Clear(0.1f, 0.1f, 0.1f, 1.0f);

			ECS::Systems::RenderScene(m_Renderer, m_Scene);

			m_Client->Render(context);

			m_Renderer.EndFrame();
			m_Window.SwapBuffers();
		}
	}
}
