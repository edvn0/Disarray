#pragma once

#include "core/events/Event.hpp"

#include <sstream>

namespace Disarray {
	class WindowResizeEvent : public Event {
	public:
		WindowResizeEvent(unsigned int width, unsigned int height)
			: w(width)
			, h(height)
		{
		}

		inline unsigned int width() const { return w; }
		inline unsigned int height() const { return h; }

		std::string to_string() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << width() << ", " << height();
			return ss.str();
		}

		MAKE_EVENT(WindowResizeEvent, WindowResize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		unsigned int w, h;
	};

	class WindowMinimizeEvent : public Event {
	public:
		WindowMinimizeEvent(bool is_minimized)
			: minimized(is_minimized)
		{
		}

		bool is_minimized() const { return minimized; }

		MAKE_EVENT(WindowMinimizeEvent, WindowMinimize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		bool minimized = false;
	};

	class WindowCloseEvent : public Event {
	public:
		WindowCloseEvent() { }

		MAKE_EVENT(WindowCloseEvent, WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class WindowTitleBarHitTestEvent : public Event {
	public:
		WindowTitleBarHitTestEvent(int x_pos, int y_pos)
			: x(x_pos)
			, y(y_pos)
		{
		}

		inline int get_x() const { return x; }
		inline int get_y() const { return y; }

		MAKE_EVENT(WindowTitleBarHitTestEvent, WindowTitleBarHitTest)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		int x;
		int y;
	};

	class AppTickEvent : public Event {
	public:
		AppTickEvent() { }

		MAKE_EVENT(AppTickEvent, AppTick)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class AppUpdateEvent : public Event {
	public:
		AppUpdateEvent() { }

		MAKE_EVENT(AppUpdateEvent, AppUpdate)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class AppRenderEvent : public Event {
	public:
		AppRenderEvent() { }

		MAKE_EVENT(AppRenderEvent, AppRender)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};
} // namespace Disarray
