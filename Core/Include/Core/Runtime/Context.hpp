#pragma once
#include <Core/Runtime/Time.hpp>
#include <Core/ECS/Scene.hpp>
#include <Core/Window/NativeWindow.hpp>
#include <Core/Project/Descriptor.hpp>
#include <Core/Input/State.hpp>
#include <Core/Scripts/Catalog.hpp>
#include <Core/ECS/SceneNodes/BuilderRegistry.hpp>
#include <Core/Graphics/Services/SceneRenderer.hpp>

namespace Core::Runtime
{
	struct Context
	{
		Context(
			const Time& time,
			Window::NativeWindow& window,
			Graphics::Gl::Renderer& renderer,
			Graphics::Services::SceneRenderer& sceneRenderService,
			const Input::State& input,
			entt::dispatcher& eventDispatcher,
			ECS::Scene& scene,
			Project::Descriptor& project)
			: time(time),
			window(window),
			renderer(renderer),
			sceneRenderService(sceneRenderService),
			input(input), 
			eventDispatcher(eventDispatcher),
			scene(scene),
			project(project) { }

		const Time& time;
		Window::NativeWindow& window;
		Graphics::Gl::Renderer& renderer;
		Graphics::Services::SceneRenderer& sceneRenderService;
		const Input::State& input;
		entt::dispatcher& eventDispatcher;
		ECS::Scene& scene;
		Project::Descriptor& project;
	};

	struct InitContext
	{
		InitContext(
			Window::NativeWindow& window, 
			entt::dispatcher& eventDispatcher, 
			Scripts::Catalog& scriptCatalog, 
			ECS::SceneNodes::BuilderRegistry& builderRegistry, 
			Project::Descriptor& project)
			: window(window), 
			eventDispatcher(eventDispatcher), 
			scriptCatalog(scriptCatalog), 
			builderRegistry(builderRegistry), 
			project(project) {}

		Window::NativeWindow& window;
		entt::dispatcher& eventDispatcher;
		Scripts::Catalog& scriptCatalog;
		ECS::SceneNodes::BuilderRegistry& builderRegistry;
		Project::Descriptor& project;
	};
}