#pragma once
#include "Events/Event.hpp"

namespace Core
{
    class KeyEvent : public Event
    {
    public:
        KeyEvent(int key, int scancode, int action, int mods) : Event(EventType::Key), m_Key(key), m_Scancode(scancode), m_Action(action), m_Mods(mods) {}

        int GetKey() const { return m_Key; }
        int GetScanCode() const { return m_Scancode; }
        int GetAction() const { return m_Action; }
        int GetMods() const { return m_Mods; }
    private:
        int m_Key;
        int m_Scancode;
        int m_Action;
        int m_Mods;
    };
}
