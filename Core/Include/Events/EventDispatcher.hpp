#pragma once
#include <vector>
#include "Events/Event.hpp"

namespace Core
{
    class EventDispatcher
    {
    public:
        void AddListener(EventCallback callback);
        void DispatchEvent(const Event& event);
    private:
        std::vector<EventCallback> m_Listeners;
    };
}
