#pragma once
#include "App/Time.hpp"
#include "ECS/Scene.hpp"
#include "Window/Window.hpp"
#include "Project/Project.hpp"
#include "Input/State.hpp"
#include "Scripts/Catalog.hpp"
#include "ECS/Resolvers/SceneResolverRegistry.hpp"
#include "Graphics/SceneRenderService.hpp"

namespace Core::App
{
	struct Context
	{
		Context(
			const Time& time,
			Window& window,
			Graphics::Gl::Renderer& renderer,
			Graphics::SceneRenderService& sceneRenderService,
			const Input::State& input,
			entt::dispatcher& eventDispatcher,
			ECS::Scene& scene,
			Project& project)
			: time(time),
			window(window),
			renderer(renderer),
			sceneRenderService(sceneRenderService),
			input(input), 
			eventDispatcher(eventDispatcher),
			scene(scene),
			project(project) { }

		const Time& time;
		Window& window;
		Graphics::Gl::Renderer& renderer;
		Graphics::SceneRenderService& sceneRenderService;
		const Input::State& input;
		entt::dispatcher& eventDispatcher;
		ECS::Scene& scene;
		Project& project;
	};

	struct InitContext
	{
		InitContext(
			Window& window, 
			entt::dispatcher& eventDispatcher, 
			Scripts::Catalog& scriptCatalog, 
			ECS::SceneResolverRegistry& resolverRegistry, 
			Project& project)
			: window(window), 
			eventDispatcher(eventDispatcher), 
			scriptCatalog(scriptCatalog), 
			resolverRegistry(resolverRegistry), 
			project(project) {}

		Window& window;
		entt::dispatcher& eventDispatcher;
		Scripts::Catalog& scriptCatalog;
		ECS::SceneResolverRegistry& resolverRegistry;
		Project& project;
	};
}