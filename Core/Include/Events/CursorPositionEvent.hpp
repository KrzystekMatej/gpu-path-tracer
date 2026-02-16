#pragma once
#include <glm/glm.hpp>
#include "Events/Event.hpp"

namespace Core
{
    class CursorPositionEvent : public Event
    {
    public:
        CursorPositionEvent(double x, double y) : Event(EventType::CursorPosition), m_Position(x, y) {}

        glm::vec2 GetPosition() const { return m_Position; }
    private:
        glm::vec2 m_Position;
    };
}
