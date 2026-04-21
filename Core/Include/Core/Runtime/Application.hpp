#pragma once
#include <filesystem>
#include <expected>
#include <Core/Window/NativeWindow.hpp>
#include <Core/Utils/Error.hpp>
#include <Core/Project/Descriptor.hpp>
#include <Core/Graphics/Gl/Renderer.hpp>
#include <Core/Graphics/Cuda/PathTracing/Renderer.hpp>
#include <Core/Assets/Manager.hpp>
#include <Core/Runtime/Time.hpp>
#include <Core/Scripts/Catalog.hpp>
#include <Core/Ecs/Scene.hpp>
#include <Core/Scripts/Runner.hpp>
#include <Core/Input/State.hpp>
#include <Core/Runtime/Layer/Stack.hpp>
#include <Core/Runtime/Layer/CommandQueue.hpp>
#include <Core/Runtime/ImGuiBackend.hpp>
#include <Core/Graphics/Services/SceneRenderer.hpp>

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

		std::expected<void, Utils::Error> SetScene(const std::filesystem::path& path);

		template <typename T, typename... Args>
		void PushLayer(std::string_view id, Args&&... args)
		{
			m_CommandQueue.PushTop<T>(id, std::forward<Args>(args)...);
		}
		
		Layer::CommandQueue& GetCommandQueue() { return m_CommandQueue; }
		void PrintInfo() const;
		void Run();

	public:
		static std::expected<std::unique_ptr<Application>, Utils::Error> Create(
			Window::Attributes windowAttributes, 
			const std::filesystem::path& projectConfigPath);

		static const Time& Time() { return s_Instance->m_Time; }
		static Window::NativeWindow& Window() { return s_Instance->m_Window; }
		static const Input::State& Input() { return s_Instance->m_Input; }
		static entt::dispatcher& EventDispatcher() { return s_Instance->m_EventDispatcher; }
		static Scripts::Catalog& ScriptCatalog() { return s_Instance->m_ScriptCatalog; }
		static Ecs::Scene& Scene() { return s_Instance->m_Scene; }
		static Ecs::BuilderRegistry& BuilderRegistry() { return s_Instance->m_BuilderRegistry; }
		static const Assets::Manager& AssetManager() { return s_Instance->m_AssetManager; }
		static const Project::Descriptor& Project() { return s_Instance->m_Project; }
		static Layer::CommandQueue& LayerCommandQueue() { return s_Instance->m_CommandQueue; }
		static entt::registry& Blackboard() { return s_Instance->m_Blackboard; }
	private:
		Application(
			Window::NativeWindow window, 
			Graphics::Gl::Renderer renderer,
			Scripts::Catalog catalog, 
			Ecs::BuilderRegistry builderRegistry,
			Assets::Manager assetManager, 
			Project::Descriptor project);

	private:
		Runtime::Time m_Time;
		Window::NativeWindow m_Window;
		ImGuiBackend m_ImGuiBackend;

		Input::State m_Input;
		entt::dispatcher m_EventDispatcher;

		Graphics::Gl::Renderer m_Renderer;

		Scripts::Catalog m_ScriptCatalog;
		Ecs::Scene m_Scene;
		Ecs::BuilderRegistry m_BuilderRegistry;
		Scripts::Runner m_ScriptRunner;

		Assets::Manager m_AssetManager;
		Project::Descriptor m_Project;

		Layer::Stack m_LayerStack;
		Layer::CommandQueue m_CommandQueue;
		entt::registry m_Blackboard;
	private:
		static Application* s_Instance;
	};
}