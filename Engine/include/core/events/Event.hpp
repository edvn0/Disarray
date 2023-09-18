#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>

namespace Disarray {

enum class EventType : std::uint8_t {
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

enum EventCategory : std::uint8_t {
	None = 0,
	EventCategoryApplication = (1 << 0),
	EventCategoryInput = (1 << 1),
	EventCategoryKeyboard = (1 << 2),
	EventCategoryMouse = (1 << 3),
	EventCategoryMouseButton = (1 << 4),
	EventCategoryScene = (1 << 5),
	EventCategoryEditor = (1 << 6)
};

constexpr auto operator&(EventCategory left, EventCategory right) -> EventCategory
{
	return static_cast<EventCategory>(static_cast<std::uint8_t>(left) & static_cast<std::uint8_t>(right));
}

constexpr auto operator|(EventCategory left, EventCategory right) -> EventCategory
{
	return static_cast<EventCategory>(static_cast<std::uint8_t>(left) | static_cast<std::uint8_t>(right));
}

#define EVENT_STATIC_CLASS_TYPE(type)                                                                                                                \
public:                                                                                                                                              \
	static auto get_static_type() -> EventType { return EventType::type; }                                                                           \
	[[nodiscard]] virtual auto get_event_type() const -> EventType final { return get_static_type(); }                                               \
	[[nodiscard]] virtual auto get_name() const -> std::string_view final { return #type; }

#define MAKE_EVENT(type, enum_type)                                                                                                                  \
public:                                                                                                                                              \
	virtual ~type() override = default;                                                                                                              \
	EVENT_STATIC_CLASS_TYPE(enum_type)

#define EVENT_CLASS_CATEGORY(category)                                                                                                               \
	virtual auto get_category_flags() const -> EventCategory final { return category; }

class Event {
public:
	bool handled = false;

	virtual ~Event() = default;
	[[nodiscard]] virtual auto get_event_type() const -> EventType = 0;
	[[nodiscard]] virtual auto get_name() const -> std::string_view = 0;
	[[nodiscard]] virtual auto get_category_flags() const -> EventCategory = 0;
	[[nodiscard]] virtual auto to_string() const -> std::string { return std::string { get_name() }; }

	[[nodiscard]] auto is_in_category(EventCategory category) const -> bool { return get_category_flags() & category; }
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

	template <HasHandledAndGetStaticType T> auto dispatch(EventCallback<T> callback) -> bool
	{
		if (event.get_event_type() == T::get_static_type() && !event.handled) {
			auto& cast = polymorphic_cast<T>(event);
			event.handled = callback(cast);
			return true;
		}
		return false;
	}

	template <HasHandledAndGetStaticType T, class Func> auto dispatch(Func&& callback) -> bool
	{
		if (event.get_event_type() == T::get_static_type() && !event.handled) {
			auto& cast = polymorphic_cast<T>(event);
			std::forward<Func>(callback)(cast);
			event.handled = false;
			return true;
		}
		return false;
	}

private:
	Event& event;
};

} // namespace Disarray
