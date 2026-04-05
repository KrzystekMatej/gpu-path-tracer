#include "Window/GLFWCallbacks.hpp"
#include <spdlog/spdlog.h>
#include "Window/Window.hpp"
#include "Events/Mouse.hpp"
#include "Events/Keyboard.hpp"
#include "Events/Window.hpp"

namespace Core
{
    namespace
    {
        constexpr std::array<Core::Input::KeyCode, GLFW_KEY_LAST + 1> BuildGlfwKeyTable()
		{
			using Core::Input::KeyCode;

			std::array<KeyCode, GLFW_KEY_LAST + 1> table{};
			table.fill(KeyCode::Unknown);

			table[GLFW_KEY_SPACE] = KeyCode::Space;
			table[GLFW_KEY_APOSTROPHE] = KeyCode::Apostrophe;
			table[GLFW_KEY_COMMA] = KeyCode::Comma;
			table[GLFW_KEY_MINUS] = KeyCode::Minus;
			table[GLFW_KEY_PERIOD] = KeyCode::Period;
			table[GLFW_KEY_SLASH] = KeyCode::Slash;

			table[GLFW_KEY_0] = KeyCode::Num0;
			table[GLFW_KEY_1] = KeyCode::Num1;
			table[GLFW_KEY_2] = KeyCode::Num2;
			table[GLFW_KEY_3] = KeyCode::Num3;
			table[GLFW_KEY_4] = KeyCode::Num4;
			table[GLFW_KEY_5] = KeyCode::Num5;
			table[GLFW_KEY_6] = KeyCode::Num6;
			table[GLFW_KEY_7] = KeyCode::Num7;
			table[GLFW_KEY_8] = KeyCode::Num8;
			table[GLFW_KEY_9] = KeyCode::Num9;

			table[GLFW_KEY_SEMICOLON] = KeyCode::Semicolon;
			table[GLFW_KEY_EQUAL] = KeyCode::Equal;

			table[GLFW_KEY_A] = KeyCode::A;
			table[GLFW_KEY_B] = KeyCode::B;
			table[GLFW_KEY_C] = KeyCode::C;
			table[GLFW_KEY_D] = KeyCode::D;
			table[GLFW_KEY_E] = KeyCode::E;
			table[GLFW_KEY_F] = KeyCode::F;
			table[GLFW_KEY_G] = KeyCode::G;
			table[GLFW_KEY_H] = KeyCode::H;
			table[GLFW_KEY_I] = KeyCode::I;
			table[GLFW_KEY_J] = KeyCode::J;
			table[GLFW_KEY_K] = KeyCode::K;
			table[GLFW_KEY_L] = KeyCode::L;
			table[GLFW_KEY_M] = KeyCode::M;
			table[GLFW_KEY_N] = KeyCode::N;
			table[GLFW_KEY_O] = KeyCode::O;
			table[GLFW_KEY_P] = KeyCode::P;
			table[GLFW_KEY_Q] = KeyCode::Q;
			table[GLFW_KEY_R] = KeyCode::R;
			table[GLFW_KEY_S] = KeyCode::S;
			table[GLFW_KEY_T] = KeyCode::T;
			table[GLFW_KEY_U] = KeyCode::U;
			table[GLFW_KEY_V] = KeyCode::V;
			table[GLFW_KEY_W] = KeyCode::W;
			table[GLFW_KEY_X] = KeyCode::X;
			table[GLFW_KEY_Y] = KeyCode::Y;
			table[GLFW_KEY_Z] = KeyCode::Z;

			table[GLFW_KEY_LEFT_BRACKET] = KeyCode::LeftBracket;
			table[GLFW_KEY_BACKSLASH] = KeyCode::Backslash;
			table[GLFW_KEY_RIGHT_BRACKET] = KeyCode::RightBracket;
			table[GLFW_KEY_GRAVE_ACCENT] = KeyCode::GraveAccent;
			table[GLFW_KEY_WORLD_1] = KeyCode::World1;
			table[GLFW_KEY_WORLD_2] = KeyCode::World2;

			table[GLFW_KEY_ESCAPE] = KeyCode::Escape;
			table[GLFW_KEY_ENTER] = KeyCode::Enter;
			table[GLFW_KEY_TAB] = KeyCode::Tab;
			table[GLFW_KEY_BACKSPACE] = KeyCode::Backspace;
			table[GLFW_KEY_INSERT] = KeyCode::Insert;
			table[GLFW_KEY_DELETE] = KeyCode::Delete;

			table[GLFW_KEY_RIGHT] = KeyCode::RightArrow;
			table[GLFW_KEY_LEFT] = KeyCode::LeftArrow;
			table[GLFW_KEY_DOWN] = KeyCode::DownArrow;
			table[GLFW_KEY_UP] = KeyCode::UpArrow;

			table[GLFW_KEY_PAGE_UP] = KeyCode::PageUp;
			table[GLFW_KEY_PAGE_DOWN] = KeyCode::PageDown;
			table[GLFW_KEY_HOME] = KeyCode::Home;
			table[GLFW_KEY_END] = KeyCode::End;
			table[GLFW_KEY_CAPS_LOCK] = KeyCode::CapsLock;
			table[GLFW_KEY_SCROLL_LOCK] = KeyCode::ScrollLock;
			table[GLFW_KEY_NUM_LOCK] = KeyCode::NumLock;
			table[GLFW_KEY_PRINT_SCREEN] = KeyCode::PrintScreen;
			table[GLFW_KEY_PAUSE] = KeyCode::Pause;

			table[GLFW_KEY_F1] = KeyCode::F1;
			table[GLFW_KEY_F2] = KeyCode::F2;
			table[GLFW_KEY_F3] = KeyCode::F3;
			table[GLFW_KEY_F4] = KeyCode::F4;
			table[GLFW_KEY_F5] = KeyCode::F5;
			table[GLFW_KEY_F6] = KeyCode::F6;
			table[GLFW_KEY_F7] = KeyCode::F7;
			table[GLFW_KEY_F8] = KeyCode::F8;
			table[GLFW_KEY_F9] = KeyCode::F9;
			table[GLFW_KEY_F10] = KeyCode::F10;
			table[GLFW_KEY_F11] = KeyCode::F11;
			table[GLFW_KEY_F12] = KeyCode::F12;
			table[GLFW_KEY_F13] = KeyCode::F13;
			table[GLFW_KEY_F14] = KeyCode::F14;
			table[GLFW_KEY_F15] = KeyCode::F15;
			table[GLFW_KEY_F16] = KeyCode::F16;
			table[GLFW_KEY_F17] = KeyCode::F17;
			table[GLFW_KEY_F18] = KeyCode::F18;
			table[GLFW_KEY_F19] = KeyCode::F19;
			table[GLFW_KEY_F20] = KeyCode::F20;
			table[GLFW_KEY_F21] = KeyCode::F21;
			table[GLFW_KEY_F22] = KeyCode::F22;
			table[GLFW_KEY_F23] = KeyCode::F23;
			table[GLFW_KEY_F24] = KeyCode::F24;
			table[GLFW_KEY_F25] = KeyCode::F25;

			table[GLFW_KEY_KP_0] = KeyCode::Keypad0;
			table[GLFW_KEY_KP_1] = KeyCode::Keypad1;
			table[GLFW_KEY_KP_2] = KeyCode::Keypad2;
			table[GLFW_KEY_KP_3] = KeyCode::Keypad3;
			table[GLFW_KEY_KP_4] = KeyCode::Keypad4;
			table[GLFW_KEY_KP_5] = KeyCode::Keypad5;
			table[GLFW_KEY_KP_6] = KeyCode::Keypad6;
			table[GLFW_KEY_KP_7] = KeyCode::Keypad7;
			table[GLFW_KEY_KP_8] = KeyCode::Keypad8;
			table[GLFW_KEY_KP_9] = KeyCode::Keypad9;
			table[GLFW_KEY_KP_DECIMAL] = KeyCode::KeypadDecimal;
			table[GLFW_KEY_KP_DIVIDE] = KeyCode::KeypadDivide;
			table[GLFW_KEY_KP_MULTIPLY] = KeyCode::KeypadMultiply;
			table[GLFW_KEY_KP_SUBTRACT] = KeyCode::KeypadSubtract;
			table[GLFW_KEY_KP_ADD] = KeyCode::KeypadAdd;
			table[GLFW_KEY_KP_ENTER] = KeyCode::KeypadEnter;
			table[GLFW_KEY_KP_EQUAL] = KeyCode::KeypadEqual;

			table[GLFW_KEY_LEFT_SHIFT] = KeyCode::LeftShift;
			table[GLFW_KEY_LEFT_CONTROL] = KeyCode::LeftControl;
			table[GLFW_KEY_LEFT_ALT] = KeyCode::LeftAlt;
			table[GLFW_KEY_LEFT_SUPER] = KeyCode::LeftSuper;
			table[GLFW_KEY_RIGHT_SHIFT] = KeyCode::RightShift;
			table[GLFW_KEY_RIGHT_CONTROL] = KeyCode::RightControl;
			table[GLFW_KEY_RIGHT_ALT] = KeyCode::RightAlt;
			table[GLFW_KEY_RIGHT_SUPER] = KeyCode::RightSuper;
			table[GLFW_KEY_MENU] = KeyCode::Menu;

			return table;
		}

		constexpr std::array<Core::Input::MouseButton, GLFW_MOUSE_BUTTON_LAST + 1> BuildGlfwMouseButtonTable()
		{
			using Core::Input::MouseButton;
			std::array<MouseButton, GLFW_MOUSE_BUTTON_LAST + 1> table{};
			table.fill(MouseButton::Unknown);
			table[GLFW_MOUSE_BUTTON_LEFT] = MouseButton::Left;
			table[GLFW_MOUSE_BUTTON_RIGHT] = MouseButton::Right;
			table[GLFW_MOUSE_BUTTON_MIDDLE] = MouseButton::Middle;
			table[GLFW_MOUSE_BUTTON_4] = MouseButton::Button4;
			table[GLFW_MOUSE_BUTTON_5] = MouseButton::Button5;
			table[GLFW_MOUSE_BUTTON_6] = MouseButton::Button6;
			table[GLFW_MOUSE_BUTTON_7] = MouseButton::Button7;
			table[GLFW_MOUSE_BUTTON_8] = MouseButton::Button8;
			return table;
		}

		constexpr auto GlfwKeyTable = BuildGlfwKeyTable();
		constexpr auto GlfwMouseButtonTable = BuildGlfwMouseButtonTable();

		Input::KeyCode TranslateGlfwKey(int glfwKey)
		{
			if (glfwKey < 0 || glfwKey > GLFW_KEY_LAST)
				return Input::KeyCode::Unknown;

			return GlfwKeyTable[static_cast<size_t>(glfwKey)];
		}

		Input::MouseButton TranslateGlfwMouseButton(int glfwButton)
		{
			if (glfwButton < 0 || glfwButton > GLFW_MOUSE_BUTTON_LAST)
				return Input::MouseButton::Unknown;

			return GlfwMouseButtonTable[static_cast<size_t>(glfwButton)];
		}

		Input::ModifierMask TranslateGlfwModifiers(int glfwMods)
		{
			Input::ModifierMask mask = 0;

			if ((glfwMods & GLFW_MOD_SHIFT) != 0)
				mask |= Input::Modifier::Shift;
			if ((glfwMods & GLFW_MOD_CONTROL) != 0)
				mask |= Input::Modifier::Control;
			if ((glfwMods & GLFW_MOD_ALT) != 0)
				mask |= Input::Modifier::Alt;
			if ((glfwMods & GLFW_MOD_SUPER) != 0)
				mask |= Input::Modifier::Super;
			if ((glfwMods & GLFW_MOD_CAPS_LOCK) != 0)
				mask |= Input::Modifier::CapsLock;
			if ((glfwMods & GLFW_MOD_NUM_LOCK) != 0)
				mask |= Input::Modifier::NumLock;

			return mask;
		}

		const char* GlfwErrorToString(int error)
		{
			switch (error)
			{
				case GLFW_NOT_INITIALIZED:     return "NOT_INITIALIZED";
				case GLFW_NO_CURRENT_CONTEXT:  return "NO_CURRENT_CONTEXT";
				case GLFW_INVALID_ENUM:        return "INVALID_ENUM";
				case GLFW_INVALID_VALUE:       return "INVALID_VALUE";
				case GLFW_OUT_OF_MEMORY:       return "OUT_OF_MEMORY";
				case GLFW_API_UNAVAILABLE:     return "API_UNAVAILABLE";
				case GLFW_VERSION_UNAVAILABLE: return "VERSION_UNAVAILABLE";
				case GLFW_PLATFORM_ERROR:      return "PLATFORM_ERROR";
				case GLFW_FORMAT_UNAVAILABLE:  return "FORMAT_UNAVAILABLE";
				case GLFW_NO_WINDOW_CONTEXT:   return "NO_WINDOW_CONTEXT";
				default:                       return "UNKNOWN";
			}
		}
    }



    void GlfwCallbacks::SetAll(GLFWwindow* windowHandle)
    {
        glfwSetKeyCallback(windowHandle, Key);
        glfwSetFramebufferSizeCallback(windowHandle, FramebufferSize);
        glfwSetMouseButtonCallback(windowHandle, MouseButton);
        glfwSetCursorPosCallback(windowHandle, CursorPosition);
		glfwSetWindowCloseCallback(windowHandle, WindowClose);
    }

    void GlfwCallbacks::Key(GLFWwindow* windowHandle, int key, int scancode, int action, int mods)
    {
        Window* window = static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));
		switch (action)
		{
			case GLFW_PRESS:
				window->m_EventRouter->OnKeyPressed(TranslateGlfwKey(key), TranslateGlfwModifiers(mods), false);
				break;
			case GLFW_RELEASE:
				window->m_EventRouter->OnKeyReleased(TranslateGlfwKey(key), TranslateGlfwModifiers(mods));
				break;
			case GLFW_REPEAT:
				window->m_EventRouter->OnKeyPressed(TranslateGlfwKey(key), TranslateGlfwModifiers(mods), true);
				break;
		}
    }

    void GlfwCallbacks::MouseButton(GLFWwindow* windowHandle, int button, int action, int mods)
    {
        Window* window = static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));

		switch (action)
		{
			case GLFW_PRESS:
				window->m_EventRouter->OnMouseButtonPressed(TranslateGlfwMouseButton(button), TranslateGlfwModifiers(mods));
				break;
			case GLFW_RELEASE:
				window->m_EventRouter->OnMouseButtonReleased(TranslateGlfwMouseButton(button), TranslateGlfwModifiers(mods));
				break;
		}
    }

    void GlfwCallbacks::CursorPosition(GLFWwindow* windowHandle, double x, double y)
    {
        Window* window = static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));
		window->m_EventRouter->OnCursorMoved(glm::vec2(static_cast<float>(x), static_cast<float>(y)));
    }

    void GlfwCallbacks::FramebufferSize(GLFWwindow* windowHandle, int width, int height)
    {
        Window* window = static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));
		window->m_Attributes.width = static_cast<uint32_t>(width);
		window->m_Attributes.height = static_cast<uint32_t>(height);
		window->m_EventRouter->OnFramebufferResized(window->m_Attributes.width, window->m_Attributes.height);
    }

	void GlfwCallbacks::WindowClose(GLFWwindow* windowHandle)
	{
		Window* window = static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));
		window->m_EventRouter->OnWindowCloseRequested();
	}

	void GlfwCallbacks::Error(int error, const char* description)
	{
		spdlog::level::level_enum level;

		switch (error)
		{
			case GLFW_OUT_OF_MEMORY:
			case GLFW_PLATFORM_ERROR:
				level = spdlog::level::critical;
				break;

			case GLFW_INVALID_ENUM:
			case GLFW_INVALID_VALUE:
				level = spdlog::level::warn;
				break;

			default:
				level = spdlog::level::err;
				break;
		}

		spdlog::log(level, "[GLFW][{}] {}", GlfwErrorToString(error), description);
	}
}
