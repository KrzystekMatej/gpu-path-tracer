#pragma once
#include "App/Time.hpp"
#include "ECS/Scene.hpp"
#include "Window/Window.hpp"
#include "Graphics/Gl/Renderer.hpp"
#include "Project/Project.hpp"
#include "Input/State.hpp"
#include "Scripts/Catalog.hpp"
#include "ECS/Resolvers/SceneResolverRegistry.hpp"

namespace Core::App
{
	struct Context
	{
		Context(const Time& time, Window& window, const Input::State& input, entt::dispatcher& eventDispatcher, Project& project)
			: time(time), window(window), input(input), eventDispatcher(eventDispatcher), project(project) { }

		const Time& time;
		Window& window;
		const Input::State& input;
		entt::dispatcher& eventDispatcher;
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