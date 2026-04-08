#include <glad/gl.h>
#include <Core/Runtime/Application.hpp>
#include <Core/Runtime/Context.hpp>
#include <Core/ECS/Systems/Render.hpp>
#include <Core/Import/SceneLoader.hpp>
#include <Core/ECS/Context.hpp>
#include <Core/ECS/Systems/Transform.hpp>

namespace Core::Runtime
{
	Application::Application(
		std::unique_ptr<UiClient> client, 
		Window::NativeWindow window,
		Graphics::Gl::Renderer renderer,
		Scripts::Catalog catalog,
		ECS::SceneNodes::BuilderRegistry builderRegistry,
		Assets::Manager assetManager, 
		Project::Descriptor project)
		: m_Client(std::move(client)), 
		m_Time(),
		m_Window(std::move(window)),
		m_Input(),
		m_EventDispatcher(),
		m_Renderer(std::move(renderer)),
		m_ScriptCatalog(std::move(catalog)),
		m_Scene(),
		m_BuilderRegistry(std::move(builderRegistry)),
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
		m_Client->Shutdown();
		m_Window.Destroy();
		Core::Window::NativeWindow::TerminateBackend();
	}

	std::expected<std::unique_ptr<Application>, Utils::Error> Application::Create
	(
		std::unique_ptr<UiClient> client, 
		Window::Attributes windowAttributes, 
		const std::filesystem::path& projectConfigPath
	)
	{
		auto windowBackendResult = Window::NativeWindow::InitBackend();
		if (!windowBackendResult)
			return std::unexpected(std::move(windowBackendResult).error());

		auto windowResult = Window::NativeWindow::Create(std::move(windowAttributes));
		if (!windowResult)
			return std::unexpected(std::move(windowResult).error());

		Window::NativeWindow window = std::move(windowResult).value();
		window.GetContext().MakeCurrent();
		window.GetContext().SetSwapInterval(1);

		Scripts::Catalog scriptCatalog;

		ECS::SceneNodes::BuilderRegistry builderRegistry;
		builderRegistry.RegisterCoreBuilders();

		auto projectResult = Project::Descriptor::Create(projectConfigPath);

		if (!projectResult)
			return std::unexpected(std::move(projectResult).error());

		Project::Descriptor project = std::move(projectResult).value();

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
			std::move(builderRegistry),
			std::move(assetManager), 
			std::move(project)));

		InitContext context(
			app->m_Window, 
			app->m_EventDispatcher, 
			app->m_ScriptCatalog,
			app->m_BuilderRegistry, 
			app->m_Project);

		app->m_Client->Init(context);
		
		auto initSceneResult = app->SetScene(app->m_Project.GetStartScenePath());

		if (!initSceneResult)
			return std::unexpected(std::move(initSceneResult).error());

		return app;
	}

	std::expected<void, Utils::Error> Application::SetScene(const std::filesystem::path& path)
	{
		auto loadedScene = Import::LoadScene(path);
		if (!loadedScene)
			return std::unexpected(std::move(loadedScene).error());
		Import::Scene scene = std::move(loadedScene).value();
		auto resolvedScene = ECS::Scene::Create(std::move(scene), m_BuilderRegistry, m_AssetManager);
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
		Window::GLVersion version = m_Window.GetContext().GetVersion();
		spdlog::info("OpenGL Version: {}.{}", version.Major, version.Minor);
		spdlog::info("Vendor: {}", version.Vendor);
		spdlog::info("Renderer: {}", version.Renderer);
		spdlog::info("GLSL: {}", version.GLSL);

		Window::GlfwVersion glfwVersion = m_Window.GetVersion();
		spdlog::info("GLFW Version: {}.{}.{}", glfwVersion.major, glfwVersion.minor, glfwVersion.revision);
	}

	void Application::Run()
	{
		Graphics::Services::SceneRenderer sceneRenderService(m_Renderer, m_AssetManager.GetStorage());
		Context appContext(m_Time, m_Window, m_Renderer, sceneRenderService, m_Input, m_EventDispatcher, m_Scene, m_Project);
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

			m_Client->BuildUi(appContext);

			m_Renderer.BindSurface(m_Window.GetRenderSurface());
			m_Renderer.Clear(0.1f, 0.1f, 0.1f, 1.0f);
			m_Client->CommitUi();

			m_Window.SwapBuffers();
		}
	}
}
