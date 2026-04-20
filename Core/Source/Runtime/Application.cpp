#include <Core/Runtime/Application.hpp>
#include <Core/Ecs/Systems/Render.hpp>
#include <Core/Import/SceneLoader.hpp>
#include <Core/Ecs/Systems/Transform.hpp>

namespace Core::Runtime
{
	Application* Application::s_Instance = nullptr;

	Application::Application(
		Window::NativeWindow window,
		Graphics::Gl::Renderer renderer,
		Scripts::Catalog catalog,
		Ecs::SceneNodes::BuilderRegistry builderRegistry,
		Assets::Manager assetManager, 
		Project::Descriptor project)
		: m_Time(),
		m_Window(std::move(window)),
		m_ImGuiBackend(),
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
		m_ImGuiBackend.Init(m_Window.GetRawHandle());
	}

	Application::~Application()
	{
		m_LayerStack.Clear();
		m_ImGuiBackend.Shutdown();
		m_Window.Destroy();
		Core::Window::NativeWindow::TerminateBackend();
	}

	std::expected<std::unique_ptr<Application>, Utils::Error> Application::Create(
		Window::Attributes windowAttributes, 
		const std::filesystem::path& projectConfigPath)
	{
		if (s_Instance)
			return std::unexpected(Utils::Error("Application instance already exists"));

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

		Ecs::SceneNodes::BuilderRegistry builderRegistry;
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
			std::move(window),
			std::move(std::move(rendererResult).value()),
			std::move(scriptCatalog),
			std::move(builderRegistry),
			std::move(assetManager), 
			std::move(project)));

		s_Instance = app.get();

		return app;
	}

	std::expected<void, Utils::Error> Application::SetScene(const std::filesystem::path& path)
	{
		auto loadedScene = Import::LoadScene(path);
		if (!loadedScene)
			return std::unexpected(std::move(loadedScene).error());
		Import::Scene scene = std::move(loadedScene).value();
		auto resolvedScene = Ecs::Scene::Create(std::move(scene), m_BuilderRegistry, m_AssetManager);
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
		m_CommandQueue.Commit(m_LayerStack);

		auto initSceneResult = SetScene(m_Project.GetStartScenePath());

		if (!initSceneResult)
			return initSceneResult.error().Log();

		m_ScriptRunner.Awake();
		while (m_Window.IsOpen())
		{
			m_Time.Update();
			m_Input.BeginFrame();

			m_Window.PollEvents();
			m_EventDispatcher.update();

			m_LayerStack.Update();

			m_ScriptRunner.Update();
			Ecs::Systems::UpdateWorldTransforms(m_Scene);

			m_CommandQueue.Commit(m_LayerStack);

			m_ImGuiBackend.BeginFrame();
			m_Renderer.BindSurface(m_Window.GetRenderSurface());
			m_Renderer.Clear(0.1f, 0.1f, 0.1f, 1.0f);
			m_LayerStack.BuildUi();
			m_ImGuiBackend.Render();

			m_LayerStack.Render(Graphics::Services::SceneRenderer(m_Renderer));

			m_Window.SwapBuffers();
		}
	}
}
