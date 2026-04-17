#include <Core/Runtime/Application.hpp>
#include <Core/Runtime/Context.hpp>
#include <Core/Ecs/Systems/Render.hpp>
#include <Core/Import/SceneLoader.hpp>
#include <Core/Ecs/Context.hpp>
#include <Core/Ecs/Systems/Transform.hpp>

namespace Core::Runtime
{
	Application::Application(
		std::unique_ptr<AppModule> appModule, 
		std::unique_ptr<UiLayer> uiLayer,
		Window::NativeWindow window,
		Graphics::Gl::Renderer renderer,
		Scripts::Catalog catalog,
		Ecs::SceneNodes::BuilderRegistry builderRegistry,
		Assets::Manager assetManager, 
		Project::Descriptor project)
		: m_AppModule(std::move(appModule)),
		m_UiLayer(std::move(uiLayer)),
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
		m_UiLayer->Shutdown();
		m_AppModule->Shutdown();
		m_Window.Destroy();
		Core::Window::NativeWindow::TerminateBackend();
	}

	std::expected<std::unique_ptr<Application>, Utils::Error> Application::Create
	(
		std::unique_ptr<AppModule> appModule, 
		std::unique_ptr<UiLayer> uiLayer,
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
			std::move(appModule),
			std::move(uiLayer),
			std::move(window),
			std::move(std::move(rendererResult).value()),
			std::move(scriptCatalog),
			std::move(builderRegistry),
			std::move(assetManager), 
			std::move(project)));

		ConfigureContext context(
			app->m_Window, 
			app->m_EventDispatcher, 
			app->m_ScriptCatalog,
			app->m_BuilderRegistry, 
			app->m_Project);

		auto configureResult = app->m_AppModule->Configure(context);
		if (!configureResult)
			return std::unexpected(std::move(configureResult).error());
		
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
		AppContext appContext(m_Time, m_Window, m_Input, m_EventDispatcher, m_Scene, m_Project);
		UiContext uiContext(m_Time, m_Window, m_Input, m_EventDispatcher, m_Scene, m_Project);
		Ecs::Context sceneContext(m_Time, m_Window, m_Input, m_EventDispatcher, m_Scene);

		m_AppModule->Start(appContext);
		m_UiLayer->Start(uiContext);

		m_ScriptRunner.Awake(sceneContext);
		while (m_Window.IsOpen())
		{
			m_Time.Update();
			m_Input.BeginFrame();

			m_Window.PollEvents();
			m_EventDispatcher.update();

			m_AppModule->Update(appContext);

			m_ScriptRunner.Update(sceneContext);
			Ecs::Systems::UpdateWorldTransforms(m_Scene);

			m_UiLayer->BuildUi(uiContext);
			RenderScene();
			m_Renderer.BindSurface(m_Window.GetRenderSurface());
			m_Renderer.Clear(0.1f, 0.1f, 0.1f, 1.0f);
			m_UiLayer->CommitUi();

			m_Window.SwapBuffers();
		}
	}

	void Application::RenderScene()
	{
		auto sceneSurfaceResult = m_AppModule->GetSceneSurface();
		if (auto sceneSurface = sceneSurfaceResult.value(); sceneSurfaceResult && sceneSurface.GetWidth() > 0 && sceneSurface.GetHeight() > 0)
		{
			m_Renderer.BindSurface(sceneSurface);
			m_Renderer.Clear(0.1f, 0.1f, 0.1f, 1.0f);
			float aspect = sceneSurface.GetWidth() / static_cast<float>(sceneSurface.GetHeight());
			Ecs::Systems::RenderScene(m_Renderer, m_Scene, m_AssetManager.GetStorage(), aspect);
		}
	}
}
