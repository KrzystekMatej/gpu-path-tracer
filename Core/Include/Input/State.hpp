#pragma once
#include <array>
#include <glm/glm.hpp>
#include "Input/Types.hpp"

namespace Core::Input
{
    class State
    {
    public:
        void BeginFrame()
        {
            m_KeyPressed.fill(false);
            m_KeyReleased.fill(false);
            m_KeyRepeated.fill(false);

            m_MousePressed.fill(false);
            m_MouseReleased.fill(false);

            m_CursorDelta = glm::vec2(0.0f);
        }

        void OnKeyPressed(KeyCode key, ModifierMask modifiers, bool repeat)
        {
            const size_t index = ToIndex(key);
            m_Modifiers = modifiers;

            if (index >= m_KeyDown.size())
                return;

            if (repeat)
            {
                m_KeyRepeated[index] = true;
                return;
            }

            if (!m_KeyDown[index])
                m_KeyPressed[index] = true;

            m_KeyDown[index] = true;
        }

        void OnKeyReleased(KeyCode key, ModifierMask modifiers)
        {
            const size_t index = ToIndex(key);
            m_Modifiers = modifiers;

            if (index >= m_KeyDown.size())
                return;

            if (m_KeyDown[index])
                m_KeyReleased[index] = true;

            m_KeyDown[index] = false;
        }

        void OnMouseButtonPressed(MouseButton button, ModifierMask modifiers)
        {
            const size_t index = ToIndex(button);
            m_Modifiers = modifiers;

            if (index >= m_MouseDown.size())
                return;

            if (!m_MouseDown[index])
                m_MousePressed[index] = true;

            m_MouseDown[index] = true;
        }

        void OnMouseButtonReleased(MouseButton button, ModifierMask modifiers)
        {
            const size_t index = ToIndex(button);
            m_Modifiers = modifiers;

            if (index >= m_MouseDown.size())
                return;

            if (m_MouseDown[index])
                m_MouseReleased[index] = true;

            m_MouseDown[index] = false;
        }

        void OnCursorMoved(glm::vec2 position)
        {
            m_CursorDelta += position - m_CursorPosition;
            m_CursorPosition = position;
        }

        bool IsKeyDown(KeyCode key) const { return Get(m_KeyDown, key); }
        bool WasKeyPressed(KeyCode key) const { return Get(m_KeyPressed, key); }
        bool WasKeyReleased(KeyCode key) const { return Get(m_KeyReleased, key); }
        bool WasKeyRepeated(KeyCode key) const { return Get(m_KeyRepeated, key); }

        bool IsMouseDown(MouseButton button) const { return Get(m_MouseDown, button); }
        bool WasMousePressed(MouseButton button) const { return Get(m_MousePressed, button); }
        bool WasMouseReleased(MouseButton button) const { return Get(m_MouseReleased, button); }

        const glm::vec2& GetCursorPosition() const { return m_CursorPosition; }
        const glm::vec2& GetCursorDelta() const { return m_CursorDelta; }

        ModifierMask GetModifiers() const { return m_Modifiers; }
        bool HasModifier(Modifier modifier) const { return Input::HasModifier(m_Modifiers, modifier); }

    private:
        template<size_t N>
        static bool Get(const std::array<bool, N>& table, KeyCode key)
        {
            const size_t index = ToIndex(key);
            return index < table.size() ? table[index] : false;
        }

        template<size_t N>
        static bool Get(const std::array<bool, N>& table, MouseButton button)
        {
            const size_t index = ToIndex(button);
            return index < table.size() ? table[index] : false;
        }

        static constexpr size_t ToIndex(KeyCode key)
        {
            return static_cast<size_t>(key);
        }

        static constexpr size_t ToIndex(MouseButton button)
        {
            return static_cast<size_t>(button);
        }

        std::array<bool, static_cast<size_t>(KeyCode::Count)> m_KeyDown{};
        std::array<bool, static_cast<size_t>(KeyCode::Count)> m_KeyPressed{};
        std::array<bool, static_cast<size_t>(KeyCode::Count)> m_KeyReleased{};
        std::array<bool, static_cast<size_t>(KeyCode::Count)> m_KeyRepeated{};

        std::array<bool, static_cast<size_t>(MouseButton::Count)> m_MouseDown{};
        std::array<bool, static_cast<size_t>(MouseButton::Count)> m_MousePressed{};
        std::array<bool, static_cast<size_t>(MouseButton::Count)> m_MouseReleased{};

        glm::vec2 m_CursorPosition{0.0f, 0.0f};
        glm::vec2 m_CursorDelta{0.0f, 0.0f};
        ModifierMask m_Modifiers = 0;
    };
}