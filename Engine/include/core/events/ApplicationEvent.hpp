#pragma once

#include "core/events/Event.hpp"

#include <sstream>

namespace Disarray {
	class WindowResizeEvent : public Event {
	public:
		MAKE_EVENT(WindowResizeEvent, WindowResize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

		WindowResizeEvent(std::uint32_t width, std::uint32_t height)
			: w(width)
			, h(height)
		{
		}

		auto width() const { return w; }
		auto height() const { return h; }

		std::string to_string() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << width() << ", " << height();
			return ss.str();
		}

	private:
		std::uint32_t w;
		std::uint32_t h;
	};

	class WindowMinimizeEvent : public Event {
	public:
		MAKE_EVENT(WindowMinimizeEvent, WindowMinimize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

		explicit WindowMinimizeEvent(bool is_minimized)
			: minimized(is_minimized)
		{
		}

		bool is_minimized() const { return minimized; }

	private:
		bool minimized = false;
	};

	class WindowCloseEvent : public Event {
	public:
		MAKE_EVENT(WindowCloseEvent, WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class WindowTitleBarHitTestEvent : public Event {
	public:
		MAKE_EVENT(WindowTitleBarHitTestEvent, WindowTitleBarHitTest)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

		WindowTitleBarHitTestEvent(int x_pos, int y_pos)
			: x(x_pos)
			, y(y_pos)
		{
		}

		auto get_x() const { return x; }
		auto get_y() const { return y; }

	private:
		int x;
		int y;
	};

	class AppTickEvent : public Event {
	public:
		MAKE_EVENT(AppTickEvent, AppTick)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class AppUpdateEvent : public Event {
	public:
		MAKE_EVENT(AppUpdateEvent, AppUpdate)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class AppRenderEvent : public Event {
	public:
		MAKE_EVENT(AppRenderEvent, AppRender)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};
} // namespace Disarray
