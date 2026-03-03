#pragma once
#include <filesystem>
#include <expected>
#include "Window/Window.hpp"
#include "Utils/Error/Error.hpp"
#include "AppClient.hpp"
#include "Project/Project.hpp"
#include "Graphics/Gl/Renderer.hpp"
#include "Assets/Manager.hpp"

namespace Core
{
	class Application
	{
	public:
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(Application&&) = delete;	

		static std::expected<std::unique_ptr<Application>, Utils::Error> Create(
			std::unique_ptr<AppClient> client, 
			WindowAttributes windowAttributes, 
			const std::filesystem::path& projectConfigPath);

		void PrintInfo() const;
		void OnEvent(const Event& event);
		void Run();
	private:
		Application(std::unique_ptr<AppClient> client, Window window, Assets::Manager assetManager, Project project);

		std::unique_ptr<AppClient> m_Client;
		Window m_Window;
		Graphics::Gl::Renderer m_Renderer;
		Assets::Manager m_AssetManager;

		Project m_Project;
	};
}