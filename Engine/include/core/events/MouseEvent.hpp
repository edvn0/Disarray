#pragma once

#include <concepts>
#include <sstream>

#include "core/MouseCode.hpp"
#include "core/events/Event.hpp"

namespace Disarray {

class MouseMovedEvent : public Event {
public:
	MouseMovedEvent(std::floating_point auto moved_x, std::floating_point auto moved_y)
		: mouse_x(moved_x)
		, mouse_y(moved_y)
	{
	}

	float get_x() const { return mouse_x; }
	float get_y() const { return mouse_y; }

	[[nodiscard]] auto to_string() const -> std::string override
	{
		std::stringstream string_stream;
		string_stream << "MouseMovedEvent: " << mouse_x << ", " << mouse_y;
		return string_stream.str();
	}

	MAKE_EVENT(MouseMovedEvent, MouseMoved)
	EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
private:
	float mouse_x, mouse_y;
};

class MouseScrolledEvent : public Event {
public:
	EVENT_STATIC_CLASS_TYPE(MouseScrolled)
	EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	MouseScrolledEvent(std::floating_point auto in_x, std::floating_point auto in_y)
		: x_offset(in_x)
		, y_offset(in_y)
	{
	}

	[[nodiscard]] auto get_x_offset() const { return x_offset; }
	[[nodiscard]] auto get_y_offset() const { return y_offset; }

	[[nodiscard]] auto to_string() const -> std::string override
	{
		std::stringstream string_stream;
		string_stream << "MouseScrolledEvent: " << get_x_offset() << ", " << get_y_offset();
		return string_stream.str();
	}

private:
	float x_offset, y_offset;
};

class MouseButtonEvent : public Event {
public:
	virtual ~MouseButtonEvent() override = default;
	[[nodiscard]] auto get_mouse_button() const -> MouseCode { return button; }

	EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
protected:
	MouseButtonEvent(MouseCode button_code)
		: button(button_code)
	{
	}

	MouseCode button;
};

class MouseButtonPressedEvent : public MouseButtonEvent {
public:
	MAKE_EVENT(MouseButtonPressedEvent, MouseButtonPressed)

	MouseButtonPressedEvent(MouseCode button_code)
		: MouseButtonEvent(button_code)
	{
	}

	[[nodiscard]] auto to_string() const -> std::string override
	{
		std::stringstream string_stream;
		string_stream << "MouseButtonPressedEvent: " << button;
		return string_stream.str();
	}
};

class MouseButtonReleasedEvent : public MouseButtonEvent {
public:
	MAKE_EVENT(MouseButtonReleasedEvent, MouseButtonReleased)

	MouseButtonReleasedEvent(MouseCode button_code)
		: MouseButtonEvent(button_code)
	{
	}

	[[nodiscard]] auto to_string() const -> std::string override
	{
		std::stringstream string_stream;
		string_stream << "MouseButtonReleasedEvent: " << button;
		return string_stream.str();
	}
};

} // namespace Disarray
