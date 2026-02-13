#include <glad/gl.h>
#include "App/Application.hpp"

namespace Core
{
	Application::Application(std::unique_ptr<AppClient> client, Window window, Project project)
		: m_Client(std::move(client)), m_Window(std::move(window)), m_Project(std::move(project)) {}

	std::expected<Application, Error> Application::Create
	(
		std::unique_ptr<AppClient> client, 
		WindowAttributes windowAttributes, 
		const std::filesystem::path& projectConfigPath
	)
	{
		auto window = Window::Create(std::move(windowAttributes));
		if (!window) {
			return std::unexpected(window.error());
		}
		
		Project project = Project::Load(projectConfigPath);

		return Application(std::move(client), std::move(window.value()), std::move(project));
	}

	void Application::PrintInfo() const
	{
		spdlog::info("OpenGL Version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
		spdlog::info("Vendor: {}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
		spdlog::info("Renderer: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
		spdlog::info("GLSL: {}", reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));

		int major, minor, revision;
		glfwGetVersion(&major, &minor, &revision);
		spdlog::info("GLFW Version: {}.{}.{}", major, minor, revision);
	}

	void Application::Run()
	{
		while (m_Window.IsOpen())
		{
			m_Window.PollEvents();
			m_Client->Update();
			m_Window.Clear();
			m_Client->Render();
			m_Window.SwapBuffers();
		}
	}
}
