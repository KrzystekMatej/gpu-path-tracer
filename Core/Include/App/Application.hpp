#pragma once
#include <filesystem>
#include <expected>
#include "Window/Window.hpp"
#include "Error/Error.hpp"
#include "AppClient.hpp"
#include "Project/Project.hpp"
#include "Graphics/Gl/Renderer.hpp"

namespace Core
{
	class Application
	{
	public:
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(Application&&) = delete;	

		static std::expected<std::unique_ptr<Application>, Error> Create
		(
			std::unique_ptr<AppClient> client, 
			WindowAttributes windowAttributes, 
			const std::filesystem::path& projectConfigPath
		);

		void PrintInfo() const;
		void OnEvent(const Event& event);
		void Run();
	private:
		Application(std::unique_ptr<AppClient> client, Window window, Project project);

		std::unique_ptr<AppClient> m_Client;
		Window m_Window;
		Graphics::Gl::Renderer m_Renderer;

		Project m_Project;
	};
}