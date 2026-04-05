#pragma once
#include <filesystem>
#include <expected>
#include "Window/Window.hpp"
#include "Utils/Error/Error.hpp"
#include "App/UiClient.hpp"
#include "Project/Project.hpp"
#include "Graphics/Gl/Renderer.hpp"
#include "Assets/Manager.hpp"
#include "App/Time.hpp"
#include "Scripts/Catalog.hpp"
#include "ECS/Scene.hpp"
#include "ECS/Systems/ScriptRunner.hpp"
#include "Input/State.hpp"

namespace Core::App
{
	class Application
	{
	public:
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(Application&&) = delete;
		~Application();

		static std::expected<std::unique_ptr<Application>, Utils::Error> Create(
			std::unique_ptr<UiClient> client, 
			WindowAttributes windowAttributes, 
			const std::filesystem::path& projectConfigPath);

		std::expected<void, Utils::Error> SetScene(const std::filesystem::path& path);
		void PrintInfo() const;
		void Run();
	private:
		Application(
			std::unique_ptr<UiClient> client, 
			Window window, 
			Graphics::Gl::Renderer renderer,
			Scripts::Catalog catalog, 
			ECS::SceneResolverRegistry resolverRegistry,
			Assets::Manager assetManager, 
			Project project);

		std::unique_ptr<UiClient> m_Client;
		Time m_Time;
		Window m_Window;

		Input::State m_Input;
		entt::dispatcher m_EventDispatcher;

		Graphics::Gl::Renderer m_Renderer;

		Scripts::Catalog m_ScriptCatalog;
		ECS::Scene m_Scene;
		ECS::SceneResolverRegistry m_ResolverRegistry;
		ECS::Systems::ScriptRunner m_ScriptRunner;

		Assets::Manager m_AssetManager;
		Project m_Project;
	};
}