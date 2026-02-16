#pragma once
#include "Events/Event.hpp"

namespace Core
{
    class MouseButtonEvent : public Event
    {
    public:
        MouseButtonEvent(int button, int action, int mods)
            : Event(EventType::MouseButton), m_Button(button), m_Action(action), m_Mods(mods) {}

        int GetButton() const { return m_Button; }
        int GetAction() const { return m_Action; }
        int GetMods() const { return m_Mods; }
    private:
        int m_Button;
        int m_Action;
        int m_Mods;
    };
}