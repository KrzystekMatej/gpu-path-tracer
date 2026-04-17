#pragma once
#include <Core/Runtime/Time.hpp>
#include <Core/Ecs/Scene.hpp>
#include <Core/Window/NativeWindow.hpp>
#include <Core/Project/Descriptor.hpp>
#include <Core/Input/State.hpp>
#include <Core/Scripts/Catalog.hpp>
#include <Core/Ecs/SceneNodes/BuilderRegistry.hpp>
#include <Core/Graphics/Services/SceneRenderer.hpp>

namespace Core::Runtime
{
	struct ConfigureContext
	{
		ConfigureContext(
			Window::NativeWindow& window,
			entt::dispatcher& eventDispatcher,
			Scripts::Catalog& scriptCatalog,
			Ecs::SceneNodes::BuilderRegistry& builderRegistry,
			Project::Descriptor& project)
			: window(window),
			eventDispatcher(eventDispatcher),
			scriptCatalog(scriptCatalog),
			builderRegistry(builderRegistry),
			project(project) {}

		Window::NativeWindow& window;
		entt::dispatcher& eventDispatcher;
		Scripts::Catalog& scriptCatalog;
		Ecs::SceneNodes::BuilderRegistry& builderRegistry;
		Project::Descriptor& project;
	};

	struct AppContext
	{
		AppContext(
			const Time& time,
			Window::NativeWindow& window,
			const Input::State& input,
			entt::dispatcher& eventDispatcher,
			Ecs::Scene& scene,
			Project::Descriptor& project)
			: time(time),
			window(window),
			input(input),
			eventDispatcher(eventDispatcher),
			scene(scene),
			project(project) {}

		const Time& time;
		Window::NativeWindow& window;
		const Input::State& input;
		entt::dispatcher& eventDispatcher;
		Ecs::Scene& scene;
		Project::Descriptor& project;
	};

	struct UiContext
	{
		UiContext(
			const Time& time,
			Window::NativeWindow& window,
			const Input::State& input,
			entt::dispatcher& eventDispatcher,
			Ecs::Scene& scene,
			Project::Descriptor& project)
			: time(time),
			window(window),
			input(input),
			eventDispatcher(eventDispatcher),
			scene(scene),
			project(project) {}

		const Time& time;
		Window::NativeWindow& window;
		const Input::State& input;
		entt::dispatcher& eventDispatcher;
		Ecs::Scene& scene;
		Project::Descriptor& project;
	};
}
