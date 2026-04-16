#pragma once
#include <filesystem>
#include <expected>
#include <Core/Window/NativeWindow.hpp>
#include <Core/Utils/Error.hpp>
#include <Core/Runtime/UiClient.hpp>
#include <Core/Project/Descriptor.hpp>
#include <Core/Graphics/Gl/Renderer.hpp>
#include <Core/Graphics/Cuda/PathTracing/Renderer.hpp>
#include <Core/Assets/Manager.hpp>
#include <Core/Runtime/Time.hpp>
#include <Core/Scripts/Catalog.hpp>
#include <Core/ECS/Scene.hpp>
#include <Core/ECS/Systems/ScriptRunner.hpp>
#include <Core/Input/State.hpp>

namespace Core::Runtime
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
			Window::Attributes windowAttributes, 
			const std::filesystem::path& projectConfigPath);

		std::expected<void, Utils::Error> SetScene(const std::filesystem::path& path);
		void PrintInfo() const;
		void Run();
	private:
		Application(
			std::unique_ptr<UiClient> client, 
			Window::NativeWindow window, 
			Graphics::Gl::Renderer renderer,
			Scripts::Catalog catalog, 
			ECS::SceneNodes::BuilderRegistry builderRegistry,
			Assets::Manager assetManager, 
			Project::Descriptor project);

		std::unique_ptr<UiClient> m_Client;
		Time m_Time;
		Window::NativeWindow m_Window;

		Input::State m_Input;
		entt::dispatcher m_EventDispatcher;

		Graphics::Gl::Renderer m_Renderer;

		Scripts::Catalog m_ScriptCatalog;
		ECS::Scene m_Scene;
		ECS::SceneNodes::BuilderRegistry m_BuilderRegistry;
		ECS::Systems::ScriptRunner m_ScriptRunner;

		Assets::Manager m_AssetManager;
		Project::Descriptor m_Project;
	};
}