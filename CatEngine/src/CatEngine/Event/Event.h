#pragma once
#include "cepch.h"

namespace CatEngine {

	enum class EventType 
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved, WindowMaximized,
		KeyPressed, KeyTyped, KeyReleased,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
	};

	enum EventCategory 
	{ 
		None = 0, 
		ApplicationEventCategory   = BIT(0),	
		InputEventCategory         = BIT(1),
		KeyboardEventCategory      = BIT(2),
		MouseEventCategory         = BIT(3),
		MouseButtonEventCategory   = BIT(4),
	};

/* The event type lets the Event Dispatcher determine what the event is and runs any premade scripts or custom scripts.
Defaults to None, but can take WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved, KeyPressed, KeyReleased, MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled events*/
#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
                               virtual EventType GetEventType() const override { return GetStaticType(); }\
                               virtual const char* GetName() const override { return #type; }
/* The event category is used to group the event into what they apply to.
This can range from CategoryApplicationEvent, CategoryInputEvent, CategoryKeyboardEvent, CategoryMouseEvent, CategoryMouseButtonEvent*/
#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	class Event {
	public:

		virtual ~Event() = default;

		bool Handled = false;

		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(EventCategory category) {
			return GetCategoryFlags() & category;
		}
	};

	class EventDispatcher {
	public:
		EventDispatcher(Event& event)
		: m_Event(event) {}

		template<typename T, typename F>
		bool Dispatch(const F& func) 
		{
			if (m_Event.GetEventType() == T::GetStaticType()) 
			{
				m_Event.Handled = func(static_cast<T&>(m_Event));
				return true;
			}
			return false;
		}
	private:
		Event& m_Event;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
}
