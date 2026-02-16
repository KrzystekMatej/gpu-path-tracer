#include "Events/EventDispatcher.hpp"


namespace Core
{
    void EventDispatcher::AddListener(EventCallback callback)
    {
        m_Listeners.push_back(std::move(callback));
    }

    void EventDispatcher::DispatchEvent(const Event& event)
    {
		for (auto& callback : m_Listeners)
		{
			callback(event);
		}
    }
}