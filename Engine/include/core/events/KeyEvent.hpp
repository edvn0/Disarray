#pragma once

#include "Event.hpp"
#include "core/KeyCode.hpp"

#include <sstream>

namespace Disarray {

class KeyEvent : public Event {
public:
	virtual ~KeyEvent() override = default;
	KeyCode get_key_code() const { return key_code; }

	EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
protected:
	KeyEvent(KeyCode kc)
		: key_code(kc)
	{
	}

	KeyCode key_code;
};

class KeyPressedEvent : public KeyEvent {
public:
	MAKE_EVENT(KeyPressedEvent, KeyPressed)

	KeyPressedEvent(KeyCode kc, int repeat)
		: KeyEvent(kc)
		, repeat_count(repeat)
	{
	}

	std::string to_string() const override
	{
		std::stringstream ss;
		ss << "KeyPressedEvent: " << key_code << " (" << repeat_count << " repeats)";
		return ss.str();
	}

	auto get_repeat_count() const { return repeat_count; }

private:
	int repeat_count;
};

class KeyReleasedEvent : public KeyEvent {
public:
	MAKE_EVENT(KeyReleasedEvent, KeyReleased)

	KeyReleasedEvent(KeyCode kc)
		: KeyEvent(kc)
	{
	}

	std::string to_string() const override
	{
		std::stringstream ss;
		ss << "KeyReleasedEvent: " << key_code;
		return ss.str();
	}
};

class KeyTypedEvent : public KeyEvent {
public:
	MAKE_EVENT(KeyTypedEvent, KeyTyped)

	KeyTypedEvent(KeyCode keycode)
		: KeyEvent(keycode)
	{
	}

	std::string to_string() const override
	{
		std::stringstream ss;
		ss << "KeyTypedEvent: " << key_code;
		return ss.str();
	}
};
} // namespace Disarray
