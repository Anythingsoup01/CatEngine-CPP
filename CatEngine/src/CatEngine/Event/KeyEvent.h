#pragma once

#include "Event.h"
#include "CatEngine/Core/KeyCodes.h"

namespace CatEngine {
	// Base Key Event Class
	class KeyEvent : public Event {
	public:
		inline KeyCode GetKeyCode() const { return m_KeyCode; }
		EVENT_CLASS_CATEGORY(InputEventCategory | KeyboardEventCategory)
	protected:
		KeyEvent(const KeyCode keycode)
		: m_KeyCode(keycode) {}
		KeyCode m_KeyCode;
	};

	// Key Pressed Event, using KeyEvent
	class KeyPressedEvent : public KeyEvent {
	public:
		KeyPressedEvent(const KeyCode keycode, bool isRepeat = false)
			: KeyEvent(keycode), m_IsRepeat(isRepeat) {}
		bool IsRepeat() const { return m_IsRepeat; }
		std::string ToString() const override {
			std::stringstream ss;
			ss << "Key Press Event " << m_KeyCode << "(repeat = " << m_IsRepeat << ")";
			return ss.str();
		}
		EVENT_CLASS_TYPE(KeyPressed)
	private:
		bool m_IsRepeat;
	};

	// Key Typed Event
	
	class KeyTypedEvent : public KeyEvent {
	public:
		KeyTypedEvent(const KeyCode keycode)
			: KeyEvent(keycode) {}
		std::string ToString() const override {
			std::stringstream ss;
			ss << "Key Typed Event " << m_KeyCode;
			return ss.str();
		}
		EVENT_CLASS_TYPE(KeyTyped)
	};

	// Key Released Event, using Key Event
	class KeyReleasedEvent : public KeyEvent {
	public:
		KeyReleasedEvent(const KeyCode keycode)
			: KeyEvent(keycode) {}
		std::string ToString() const override {
			std::stringstream ss;
			ss << "Key Release Event " << m_KeyCode;
			return ss.str();
		}
		EVENT_CLASS_TYPE(KeyReleased)
	};
}
