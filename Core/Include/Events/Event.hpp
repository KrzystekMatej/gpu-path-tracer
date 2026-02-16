#pragma once
#include <functional>

namespace Core
{
	enum class EventType
	{
		FrameBufferSize,
		MouseButton,
		CursorPosition,
		Key,
	};

	class Event
	{
	public:
		Event(EventType type) : m_Type(type) {}
		EventType GetType() const { return m_Type; }
	private:
		EventType m_Type;
	};

    using EventCallback = std::function<void(const Event&)>;
}