#pragma once

#include "Forward.hpp"

#include <glm/common.hpp>
#include <glm/detail/qualifier.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fmt/core.h>
#include <magic_enum.hpp>

#include <array>
#include <concepts>
#include <filesystem>
#include <functional>
#include <limits>
#include <unordered_map>
#include <unordered_set>

#include "core/Collections.hpp"
#include "core/Concepts.hpp"
#include "core/Hashes.hpp"
#include "core/Input.hpp"
#include "core/Log.hpp"
#include "core/Window.hpp"
#include "graphics/Image.hpp"
#include "graphics/Shader.hpp"
#include "graphics/Texture.hpp"

extern "C" {
struct ImRect;
}

namespace Disarray::UI {

namespace Detail {
	template <class Formatter, class T>
	concept ArgumentFormatter = requires(const Formatter& formatter, const T& value) {
		{
			formatter.operator()(value)
		} -> std::same_as<std::string>;
	};

	template <class T> struct DefaultFormatter {
		auto operator()(const T& value) const -> std::string { return fmt::format("{}", value); }
	};
} // namespace Detail

using ExtensionSet = Collections::StringSet;
using ImageIdentifier = std::uint64_t;

class InterfaceCaches {
	using ImageCache = std::unordered_map<ImageIdentifier, std::unique_ptr<ImageIdentifier>>;

public:
	static void initialise();
	static void destruct();

	static auto descriptor_cache() -> auto& { return image_descriptor_cache; }
	static auto font_cache() -> auto& { return font_map; }

private:
	static inline ImageCache image_descriptor_cache {};
	static inline Collections::StringMap<ImFont*> font_map {};
};

class Scope {
	DISARRAY_MAKE_NONCOPYABLE(Scope);

public:
	explicit Scope(std::string_view name);
	~Scope();
};

static constexpr std::array<glm::vec2, 2> default_uvs = { glm::vec2 { 0.f, 0.f }, glm::vec2 { 1.f, 1.f } };

using UIFunction = std::function<void(void)>;
static constexpr auto default_function = []() {};

void text(const std::string&);

template <typename... Args> void text(fmt::format_string<Args...> fmt_string, Args&&... args)
{
	auto formatted = fmt::format(fmt_string, std::forward<Args>(args)...);
	text(formatted);
}

void text_wrapped(const std::string&);

template <typename... Args> void text_wrapped(fmt::format_string<Args...> fmt_string, Args&&... args)
{
	auto formatted = fmt::format(fmt_string, std::forward<Args>(args)...);
	text_wrapped(formatted);
}

static constexpr inline auto button_size = 64;

bool image_button(const Disarray::Image&, glm::vec2 size = { button_size, button_size }, const std::array<glm::vec2, 2>& uvs = default_uvs);
void image(const Disarray::Image&, glm::vec2 size = { button_size, button_size }, const std::array<glm::vec2, 2>& uvs = default_uvs);
bool image_button(const Disarray::Texture&, glm::vec2 size = { button_size, button_size }, const std::array<glm::vec2, 2>& uvs = default_uvs);
void image(const Disarray::Texture&, glm::vec2 size = { button_size, button_size }, const std::array<glm::vec2, 2>& uvs = default_uvs);

namespace Tabular {
	auto table(std::string_view name, const Collections::StringViewMap<std::string>& map) -> bool;

	template <class T>
	auto table(std::string_view name, const Collections::StringViewMap<T>& map, Detail::ArgumentFormatter<T> auto formatter) -> bool
	{
		Collections::StringViewMap<std::string> temporary {};
		for (const auto& [key, value] : map) {
			temporary.try_emplace(key, formatter(value));
		}
		return table(name, temporary);
	}

	template <class T> auto table(std::string_view name, const Collections::StringViewMap<T>& map) -> bool
	{
		Collections::StringViewMap<std::string> temporary {};
		auto formatter = Detail::DefaultFormatter<T> {};
		for (const auto& [key, value] : map) {
			temporary.try_emplace(key, formatter(value));
		}
		return table(name, temporary);
	}
} // namespace Tabular
namespace Popup {
	auto select_file(const ExtensionSet& file_types, const std::filesystem::path& base = "Assets") -> std::optional<std::filesystem::path>;
}
namespace Input {
	auto general_slider(std::string_view name, int count, float* base, float min, float max) -> bool;

	template <std::size_t N = 1, std::floating_point T = float>
		requires(N > 0 && N <= 4)
	auto slider(std::string_view name, T* base, T min = 0, T max = 1) -> bool
	{
		return general_slider(name, N, base, min, max);
	}

	template <std::floating_point T, int N> auto slider(std::string_view name, glm::vec<N, T>& vec, T min = 0, T max = 1) -> bool
	{
		return general_slider(name, N, glm::value_ptr(vec), min, max);
	}

	auto general_drag(std::string_view name, int count, float* base, float velocity, float min, float max) -> bool;

	template <std::size_t N, std::floating_point T>
		requires(N >= 0 && N <= 4)
	auto drag(std::string_view name, T* base, T velocity = T { 1 }, T min = 0, T max = 1) -> bool
	{
		return general_drag(name, N, base, velocity, min, max);
	}

	template <std::floating_point T, int N> auto drag(std::string_view name, glm::vec<N, T>& vec, T velocity = T { 1 }, T min = 0, T max = 1) -> bool
	{
		return general_drag(name, N, glm::value_ptr(vec), velocity, min, max);
	}

	auto general_input(std::string_view name, int count, float* base, float velocity, float min, float max) -> bool;

	template <std::size_t N = 1, std::floating_point T = float>
		requires(N >= 0 && N <= 4)
	auto input(std::string_view name, T* base, T velocity = T { 1 }, T min = 0, T max = 1) -> bool
	{
		return general_input(name, N, base, velocity, min, max);
	}

	template <std::floating_point T, int N> auto input(std::string_view name, glm::vec<N, T>& vec, T velocity = T { 1 }, T min = 0, T max = 1) -> bool
	{
		return general_input(name, N, glm::value_ptr(vec), velocity, min, max);
	}

	auto general_input(std::string_view name, int count, std::uint32_t* base, std::uint32_t min, std::uint32_t max) -> bool;
	auto general_input(std::string_view name, int count, std::uint64_t* base, std::uint64_t min, std::uint64_t max) -> bool;
	auto general_input(std::string_view name, int count, std::int32_t* base, std::int32_t min, std::int32_t max) -> bool;
	auto general_input(std::string_view name, int count, std::int64_t* base, std::int64_t min, std::int64_t max) -> bool;

	template <std::size_t N = 1, std::integral T = std::uint32_t>
		requires(N >= 0 && N <= 4)
	auto input(std::string_view name, T* base, T min = 0, T max = 1) -> bool
	{
		return general_input(name, N, base, min, max);
	}

	template <std::integral T, int N> auto input(std::string_view name, glm::vec<N, T>& vec, T min = 0, T max = 1) -> bool
	{
		return general_input(name, N, glm::value_ptr(vec), min, max);
	}

} // namespace Input

void scope(std::string_view name, UIFunction&& func = default_function);

void begin(std::string_view);
void end();

void shift_cursor_y(float by);
void shift_cursor_y(std::floating_point auto by) { shift_cursor_y(static_cast<float>(by)); }

auto begin_combo(std::string_view name, std::string_view data) -> bool;
void end_combo();
auto is_selectable(std::string_view name, bool is_selected) -> bool;
void set_item_default_focus();

void drag_drop(const std::filesystem::path& path);
auto accept_drag_drop(const std::string& payload_identifier, const ExtensionSet& allowed_extension = { "*" }) -> std::optional<std::filesystem::path>;
auto is_item_hovered() -> bool;
auto is_mouse_double_clicked(MouseCode code = MouseCode::Left) -> bool;
void handle_double_click(auto&& handler)
{
	if (is_item_hovered() && is_mouse_double_clicked()) {
		handler();
	}
}

/**
 * Create a enum dependent combo choice. Returns true if changed.
 * @tparam T
 * @param name
 * @param initial_value reference to value
 * @return true if changed
 */
template <IsEnum T> auto combo_choice(std::string_view name, T& initial_value) -> bool
{
	const std::string as_string { name };
	using namespace std::string_view_literals;

	const auto values_for_t = magic_enum::enum_values<T>();
	const auto& preview_value = initial_value;
	auto new_value = preview_value;
	if (begin_combo(as_string.c_str(), magic_enum::enum_name(preview_value))) {
		for (const auto& value : values_for_t) {
			const bool is_selected = (value == preview_value);
			if (is_selectable(magic_enum::enum_name(value), is_selected)) {
				new_value = value;
			}

			if (is_selected) {
				set_item_default_focus();
			}
		}
		end_combo();
	}

	const auto changed = new_value != preview_value;
	initial_value = new_value;
	return changed;
}

template <IsEnum T> auto combo_choice(std::string_view name, std::reference_wrapper<T> initial_value) -> bool
{
	const std::string as_string { name };
	using namespace std::string_view_literals;

	const auto values_for_t = magic_enum::enum_values<T>();
	const auto& preview_value = initial_value.get();
	auto& base_value = initial_value.get();
	auto new_value = preview_value;
	if (begin_combo(as_string.c_str(), magic_enum::enum_name(preview_value))) {
		for (const auto& value : values_for_t) {
			const bool is_selected = (value == preview_value);
			if (is_selectable(magic_enum::enum_name(value), is_selected)) {
				new_value = value;
			}

			if (is_selected) {
				set_item_default_focus();
			}
		}
		end_combo();
	}

	const auto changed = new_value != preview_value;
	base_value = new_value;
	return changed;
}

auto checkbox(const std::string&, bool&) -> bool;

auto begin_menu_bar(const ImRect&) -> bool;
auto end_menu_bar() -> void;

auto shader_drop_button(Device&, const std::string& button_name, ShaderType shader_type, Ref<Shader>& out_shader) -> bool;
auto texture_drop_button(Device&, const Texture& texture) -> Ref<Disarray::Texture>;
auto shader_drop_button(const Device&, const std::string& button_name, ShaderType shader_type, Ref<Shader>& out_shader) -> bool;
auto texture_drop_button(const Device&, const Texture& texture) -> Ref<Disarray::Texture>;

auto is_maximised(Window& window) -> bool;
void remove_image(const Texture& texture);
void remove_image(ImageIdentifier hash);

} // namespace Disarray::UI
