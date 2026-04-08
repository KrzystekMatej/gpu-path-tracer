#pragma once
#include <entt/entt.hpp>
#include <Core/Events/Keyboard.hpp>
#include <Core/Events/Mouse.hpp>
#include <Core/Events/Window.hpp>
#include <Core/Input/State.hpp>

namespace Core::Events
{
    class WindowEventRouter
    {
    public:
        WindowEventRouter(Input::State& input, entt::dispatcher& dispatcher)
            : m_Input(input), m_Dispatcher(dispatcher) {}

        void OnKeyPressed(Input::KeyCode key, Input::ModifierMask modifiers, bool repeat)
        {
            m_Input.OnKeyPressed(key, modifiers, repeat);
            m_Dispatcher.enqueue<Events::KeyPressed>(key, modifiers, repeat);
        }

        void OnKeyReleased(Input::KeyCode key, Input::ModifierMask modifiers)
        {
            m_Input.OnKeyReleased(key, modifiers);
            m_Dispatcher.enqueue<Events::KeyReleased>(key, modifiers);
        }

        void OnMouseButtonPressed(Input::MouseButton button, Input::ModifierMask modifiers)
        {
            m_Input.OnMouseButtonPressed(button, modifiers);
            m_Dispatcher.enqueue<Events::MouseButtonPressed>(button, modifiers);
        }

        void OnMouseButtonReleased(Input::MouseButton button, Input::ModifierMask modifiers)
        {
            m_Input.OnMouseButtonReleased(button, modifiers);
            m_Dispatcher.enqueue<Events::MouseButtonReleased>(button, modifiers);
        }

        void OnCursorMoved(glm::vec2 position)
        {
            m_Input.OnCursorMoved(position);
            m_Dispatcher.enqueue<Events::CursorMoved>(position);
        }

        void OnFramebufferResized(uint32_t width, uint32_t height)
        {
            m_Dispatcher.enqueue<Events::FramebufferResized>(width, height);
        }

        void OnWindowCloseRequested()
        {
            m_Dispatcher.enqueue<Events::WindowCloseRequested>();
        }

    private:
        Input::State& m_Input;
        entt::dispatcher& m_Dispatcher;
    };
}
