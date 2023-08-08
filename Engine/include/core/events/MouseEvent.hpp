#pragma once

#include "core/MouseCode.hpp"
#include "core/events/Event.hpp"

#include <sstream>

namespace Disarray {

	class MouseMovedEvent : public Event {
	public:
		MouseMovedEvent(float x, float y)
			: mouse_x(x)
			, mouse_y(y)
		{
		}

		float get_x() const { return mouse_x; }
		float get_y() const { return mouse_y; }

		std::string to_string() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << mouse_x << ", " << mouse_y;
			return ss.str();
		}

		MAKE_EVENT(MouseMovedEvent, MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
		float mouse_x, mouse_y;
	};

	class MouseScrolledEvent : public Event {
	public:
		MouseScrolledEvent(float x, float y)
			: x_offset(x)
			, y_offset(y)
		{
		}

		float get_x_offset() const { return x_offset; }
		float get_y_offset() const { return y_offset; }

		std::string to_string() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << get_x_offset() << ", " << get_y_offset();
			return ss.str();
		}

		EVENT_STATIC_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
		float x_offset, y_offset;
	};

	class MouseButtonEvent : public Event {
	public:
		MouseCode get_mouse_button() const { return button; }

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
		MouseButtonPressedEvent(MouseCode button_code)
			: MouseButtonEvent(button_code)
		{
		}

		std::string to_string() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << button;
			return ss.str();
		}

		EVENT_STATIC_CLASS_TYPE(MouseButtonPressed)
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent {
	public:
		MouseButtonReleasedEvent(MouseCode button_code)
			: MouseButtonEvent(button_code)
		{
		}

		std::string to_string() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << button;
			return ss.str();
		}

		EVENT_STATIC_CLASS_TYPE(MouseButtonReleased)
	};

} // namespace Disarray
