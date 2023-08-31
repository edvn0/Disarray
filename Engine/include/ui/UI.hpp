#pragma once

#include <glm/glm.hpp>

#include <fmt/core.h>
#include <magic_enum.hpp>

#include <array>
#include <filesystem>
#include <functional>
#include <unordered_map>
#include <unordered_set>

#include "Forward.hpp"
#include "core/Collections.hpp"
#include "core/Concepts.hpp"
#include "core/Hashes.hpp"
#include "core/Input.hpp"
#include "core/Window.hpp"
#include "graphics/Image.hpp"
#include "graphics/Shader.hpp"
#include "graphics/Texture.hpp"

namespace Disarray::UI {

using ExtensionSet = Collections::StringSet;
using ImageIdentifier = std::uint64_t;

class InterfaceCaches {
	using ImageCache = std::unordered_map<ImageIdentifier, std::unique_ptr<ImageIdentifier>>;

public:
	static void initialise();
	static void destruct();

	static auto& descriptor_cache() { return image_descriptor_cache; }
	static auto& font_cache() { return font_map; }

private:
	inline static ImageCache image_descriptor_cache {};
	inline static Collections::StringMap<ImFont*> font_map {};
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

void image_button(Image&, glm::vec2 size = { 64, 64 }, const std::array<glm::vec2, 2>& uvs = default_uvs);
void image_button(const Image&, glm::vec2 size = { 64, 64 }, const std::array<glm::vec2, 2>& uvs = default_uvs);
void image(Image&, glm::vec2 size = { 64, 64 }, const std::array<glm::vec2, 2>& uvs = default_uvs);
void image_button(Texture&, glm::vec2 size = { 64, 64 }, const std::array<glm::vec2, 2>& uvs = default_uvs);
void image(Texture&, glm::vec2 size = { 64, 64 }, const std::array<glm::vec2, 2>& uvs = default_uvs);

void scope(std::string_view name, UIFunction&& func = default_function);

void begin(std::string_view);
void end();

bool begin_combo(std::string_view name, std::string_view data);
void end_combo();
bool is_selectable(std::string_view name, const bool is_selected);
void set_item_default_focus();

void drag_drop(const std::filesystem::path& path);
std::optional<std::filesystem::path> accept_drag_drop(const std::string& payload_identifier, const ExtensionSet& allowed_extension = { "*" });
bool is_item_hovered();
bool is_mouse_double_clicked(MouseCode code = MouseCode::Left);
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

bool checkbox(const std::string&, bool&);

bool shader_drop_button(Device&, const std::string& button_name, ShaderType shader_type, Ref<Shader>& out_shader);
Ref<Texture> texture_drop_button(Device&, const Texture& texture);

bool is_maximised(Window& window);
void remove_image(const Texture& texture);
void remove_image(ImageIdentifier hash);

} // namespace Disarray::UI
