#include <Core/Runtime/Application.hpp>
#include <Core/Graphics/Ecs/Render.hpp>
#include <Core/Import/SceneLoader.hpp>
#include <Core/Ecs/Transform.hpp>
#include <Core/Capture/MotionRecorder.hpp>

namespace Core::Runtime
{
	Application* Application::s_Instance = nullptr;

	Application::Application(
		Window::NativeWindow window,
		Graphics::Gl::Renderer renderer,
		Scripts::Catalog catalog,
		Ecs::BuilderRegistry builderRegistry,
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
#ifdef CORE_DEBUG
		spdlog::set_level(spdlog::level::trace);
#else
		spdlog::set_level(spdlog::level::info);
#endif

		if (s_Instance)
			return std::unexpected(Utils::Error("Application instance already exists"));

		CORE_TRY_DISCARD_CONTEXT(Window::NativeWindow::InitBackend(), "Failed to initialize window backend");
		CORE_TRY_CONTEXT(window, Window::NativeWindow::Create(std::move(windowAttributes)), "Failed to create window");
		window.GetContext().MakeCurrent();
		window.GetContext().SetSwapInterval(1);

		Scripts::Catalog scriptCatalog;

		Ecs::BuilderRegistry builderRegistry;
		CORE_TRY_DISCARD_CONTEXT(builderRegistry.RegisterCoreBuilders(), "Failed to register core builders");

		CORE_TRY_CONTEXT(project, Project::Descriptor::Create(projectConfigPath), "Failed to create project descriptor");
		CORE_TRY_CONTEXT(assetManager, Assets::Manager::Create(project.GetContentPath()), "Failed to create asset manager");
		CORE_TRY_CONTEXT(renderer, Graphics::Gl::Renderer::Create(assetManager), "Failed to create renderer");

		std::unique_ptr<Application> app = std::unique_ptr<Application>(new Application(
			std::move(window),
			std::move(renderer),
			std::move(scriptCatalog),
			std::move(builderRegistry),
			std::move(assetManager), 
			std::move(project)));

		s_Instance = app.get();

		return app;
	}

	std::expected<void, Utils::Error> Application::SetScene(const std::filesystem::path& path)
	{
		CORE_TRY_CONTEXT(sceneSource, Import::LoadScene(path), "Failed to load scene");
		CORE_TRY_CONTEXT(scene, Ecs::Scene::Create(std::move(sceneSource), m_BuilderRegistry, m_AssetManager), "Failed to create scene");
		m_Scene = std::move(scene);

		Ecs::UpdateWorldTransforms(m_Scene);

		CORE_TRY_DISCARD_CONTEXT(m_ScriptRunner.Bind(m_ScriptCatalog, m_Scene), "Failed to bind script runner");
		m_EventDispatcher.trigger(Ecs::SceneChangedEvent());
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

	std::expected<void, Utils::Error> Application::Run()
	{
		m_CommandQueue.Commit(m_LayerStack);

		CORE_TRY_DISCARD_CONTEXT(SetScene(m_Project.GetStartScenePath()), "Failed to initialize start scene");

		m_ScriptRunner.Awake();
		while (m_Window.IsOpen())
		{
			m_Time.Update();
			m_Input.BeginFrame();

			m_Window.PollEvents();
			m_EventDispatcher.update();

			m_LayerStack.Update();

			m_ScriptRunner.Update();
			Ecs::UpdateWorldTransforms(m_Scene);
			Capture::UpdateMotionRecording(m_Scene, m_Time.GetDeltaTime());

			m_CommandQueue.Commit(m_LayerStack);

			m_ImGuiBackend.BeginFrame();
			m_LayerStack.BuildUi();
			
			m_LayerStack.Render(Graphics::Services::SceneRenderer(m_Renderer));

			m_Renderer.BindSurface(m_Window.GetRenderSurface());
			m_Renderer.Clear(0.0f, 0.0f, 0.0f, 1.0f);
			m_ImGuiBackend.Render();


			m_Window.SwapBuffers();
		}
		return {};
	}
}
