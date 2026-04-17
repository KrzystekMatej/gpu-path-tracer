#pragma once
#include <Core/Runtime/Time.hpp>
#include <Core/Ecs/Scene.hpp>
#include <Core/Window/NativeWindow.hpp>
#include <Core/Input/State.hpp>	

namespace Core::Ecs
{
	struct Context
	{
		Context(const Runtime::Time& time, Window::NativeWindow& window, const Input::State& input, entt::dispatcher& eventDispatcher, Scene& scene)
			: time(time), window(window), input(input), eventDispatcher(eventDispatcher), scene(scene) { }

		const Runtime::Time& time;
		Window::NativeWindow& window;
		const Input::State& input;
		entt::dispatcher& eventDispatcher;
		Scene& scene;
	};
}
