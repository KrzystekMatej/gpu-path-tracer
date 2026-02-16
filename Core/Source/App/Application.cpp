#include <glad/gl.h>
#include "App/Application.hpp"
#include "App/AppContext.hpp"

namespace Core
{
	Application::Application(std::unique_ptr<AppClient> client, Window window, Project project)
		: m_Client(std::move(client)), m_Window(std::move(window)), m_Renderer(m_Window.GetContext()), m_Project(std::move(project))
	{ 
		m_Renderer.Initialize();
		m_Window.InitCallbacks();
		m_Window.SetEventCallback([this](const Event& event) { OnEvent(event); });
	}

	std::expected<std::unique_ptr<Application>, Error> Application::Create
	(
		std::unique_ptr<AppClient> client, 
		WindowAttributes windowAttributes, 
		const std::filesystem::path& projectConfigPath
	)
	{
		if (!glfwInit())
			return std::unexpected(Error("Failed to initialize GLFW!"));

		auto windowResult = Window::Create(std::move(windowAttributes));
		if (!windowResult)
			return std::unexpected(windowResult.error());

		Window window = std::move(windowResult.value());

		window.GetContext().MakeCurrent();
		if (!gladLoadGL(glfwGetProcAddress))
			return std::unexpected(Error("Failed to initialize GLAD!"));
		window.GetContext().SetSwapInterval(1);

		Project project = Project::Load(projectConfigPath);

		return std::unique_ptr<Application>(new Application(std::move(client), std::move(window), std::move(project)));
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
		m_Client->OnEvent(AppContext(m_Window, m_Renderer, m_Project), event);
	}

	void Application::Run()
	{
		while (m_Window.IsOpen())
		{
			m_Window.PollEvents();

			m_Renderer.BeginFrame();
			m_Renderer.Clear(0.1f, 0.1f, 0.1f, 1.0f);

			AppContext context(m_Window, m_Renderer, m_Project);
			m_Client->Update(context);
			m_Client->Render(context);
			m_Renderer.EndFrame();

			m_Window.SwapBuffers();
		}
	}
}
