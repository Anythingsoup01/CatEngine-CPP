#pragma once

#include "Event.h"

namespace CatEngine {
	// Window Close Event
	class WindowCloseEvent : public Event {
	public:
		WindowCloseEvent() {}
		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(ApplicationEventCategory)
	};
	// Window Resize Event
	class WindowResizeEvent : public Event {
	public:
		WindowResizeEvent(unsigned int width, unsigned int height)
			: m_Width(width), m_Height(height) {}

		inline unsigned int GetWidth() const { return m_Width; }
		inline unsigned int GetHeight() const { return m_Height; }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "Window Resize Event: " << m_Width << ", " << m_Height;
			return ss.str();
		}
		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(ApplicationEventCategory)
	private:
		unsigned int m_Width, m_Height;
	};
	// Window Focus Event
	class WindowFocusEvent : public Event {
	public:
		WindowFocusEvent() {}
		EVENT_CLASS_TYPE(WindowFocus)
		EVENT_CLASS_CATEGORY(ApplicationEventCategory)
	};
	// Window Lost Focus Event
	class WindowLostFocusEvent : public Event {
	public:
		WindowLostFocusEvent() {}
		EVENT_CLASS_TYPE(WindowLostFocus)
		EVENT_CLASS_CATEGORY(ApplicationEventCategory)
	};
	// Window Moved Event
	class WindowMovedEvent : public Event {
	public:
		WindowMovedEvent() {}
		EVENT_CLASS_TYPE(WindowMoved)
		EVENT_CLASS_CATEGORY(ApplicationEventCategory)
	};
	// Window Maximized
	class WindowMaximizedEvent : public Event {
	public:
		WindowMaximizedEvent() {}
		EVENT_CLASS_TYPE(WindowMaximized)
		EVENT_CLASS_CATEGORY(ApplicationEventCategory)
	};
}
