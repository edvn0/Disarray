#pragma once

#include <string>
#include <string_view>

namespace Disarray {

enum class EventType {
	None = 0,
	WindowClose,
	WindowMinimize,
	WindowResize,
	WindowFocus,
	WindowLostFocus,
	WindowMoved,
	WindowTitleBarHitTest,
	AppTick,
	AppUpdate,
	AppRender,
	KeyPressed,
	KeyReleased,
	KeyTyped,
	MouseButtonPressed,
	MouseButtonReleased,
	MouseMoved,
	MouseScrolled,
	ScenePreStart,
	ScenePostStart,
	ScenePreStop,
	ScenePostStop,
	EditorExitPlayMode,
	SelectionChanged
};

enum EventCategory {
	None = 0,
	EventCategoryApplication = (1 << 0),
	EventCategoryInput = (1 << 1),
	EventCategoryKeyboard = (1 << 2),
	EventCategoryMouse = (1 << 3),
	EventCategoryMouseButton = (1 << 4),
	EventCategoryScene = (1 << 5),
	EventCategoryEditor = (1 << 6)
};

#define EVENT_STATIC_CLASS_TYPE(type)                                                                                                                \
public:                                                                                                                                              \
	static EventType get_static_type() { return EventType::type; }                                                                                   \
	virtual EventType get_event_type() const final { return get_static_type(); }                                                                     \
	virtual std::string_view get_name() const final { return #type; }

#define MAKE_EVENT(type, enum_type)                                                                                                                  \
public:                                                                                                                                              \
	virtual ~type() override = default;                                                                                                              \
	EVENT_STATIC_CLASS_TYPE(enum_type)

#define EVENT_CLASS_CATEGORY(category)                                                                                                               \
	virtual int get_category_flags() const final { return category; }

class Event {
public:
	bool handled = false;

	virtual ~Event() = default;
	virtual EventType get_event_type() const = 0;
	virtual std::string_view get_name() const = 0;
	virtual int get_category_flags() const = 0;
	virtual std::string to_string() const { return std::string { get_name() }; }

	inline bool is_in_category(EventCategory category) const { return get_category_flags() & category; }
};

template <typename T>
concept HasHandledAndGetStaticType = requires(T t) {
	t.handled;
	{
		T::get_static_type()
	} -> std::same_as<EventType>;
};

class EventDispatcher {
	template <HasHandledAndGetStaticType T> using EventCallback = std::function<bool(T&)>;

public:
	explicit EventDispatcher(Event& consumed_event)
		: event(consumed_event)
	{
	}

	template <HasHandledAndGetStaticType T> bool dispatch(EventCallback<T> callback)
	{
		if (event.get_event_type() == T::get_static_type() && !event.handled) {
			auto& cast = polymorphic_cast<T>(event);
			event.handled = callback(cast);
			return true;
		}
		return false;
	}

private:
	Event& event;
};

inline std::ostream& operator<<(std::ostream& os, const Event& e) { return os << e.to_string(); }
} // namespace Disarray
