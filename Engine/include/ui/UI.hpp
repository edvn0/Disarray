#pragma once

#include "Forward.hpp"
#include "core/Concepts.hpp"
#include "core/Input.hpp"
#include "core/Window.hpp"
#include "graphics/Image.hpp"
#include "graphics/Shader.hpp"
#include "graphics/Texture.hpp"

#include <array>
#include <filesystem>
#include <functional>
#include <glm/glm.hpp>
#include <magic_enum.hpp>
#include <unordered_map>
#include <unordered_set>

namespace Disarray::UI {

using ImageIdentifier = std::uint64_t;
class DescriptorCache {
	using ImageCache = std::unordered_map<ImageIdentifier, std::unique_ptr<ImageIdentifier>>;

public:
	static void initialise();
	static void destruct();

	static auto& get_cache() { return cache; }

private:
	inline static ImageCache cache {};
};

static constexpr std::array<glm::vec2, 2> default_uvs = { glm::vec2 { 0.f, 0.f }, glm::vec2 { 1.f, 1.f } };

using UIFunction = std::function<void(void)>;
static constexpr auto default_function = []() {};

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
std::optional<std::filesystem::path> accept_drag_drop(
	const std::string& payload_identifier, const std::unordered_set<const char*>& allowed_extension = { "*" });
bool is_item_hovered();
bool is_mouse_double_clicked(MouseCode code = MouseCode::Left);
void handle_double_click(auto&& handler)
{
	if (is_item_hovered() && is_mouse_double_clicked()) {
		handler();
	}
}

/**
 * Create a enum dependent combo choice. Returns [true_if_changed, current_or_new_value].
 * @tparam T
 * @param name
 * @param initial_value reference to value
 * @return true if changed
 */
template <IsEnum T> bool combo_choice(std::string name, T& initial_value)
{
	using namespace std::string_view_literals;

	const auto values_for_t = magic_enum::enum_values<T>();
	const auto& preview_value = initial_value;
	auto new_value = preview_value;
	if (begin_combo(name.c_str(), magic_enum::enum_name(preview_value))) {
		for (const auto& value : values_for_t) {
			const bool is_selected = (value == preview_value);
			if (is_selectable(magic_enum::enum_name(value), is_selected))
				new_value = value;

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
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

bool shader_drop_button(Device&, const std::string& button_name, ShaderType shader_type, Ref<Shader>& out_shader);
bool texture_drop_button(Device&, const std::string& button_name, const Texture&, Ref<Texture>& out_texture);

bool is_maximised(Window& window);

} // namespace Disarray::UI
