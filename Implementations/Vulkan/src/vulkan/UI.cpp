#include "DisarrayPCH.hpp"

#include "Forward.hpp"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_vulkan.h>
#include <vulkan/vulkan.h>

#include <imgui.h>
#include <tinyfiledialogs.h>

#include <filesystem>
#include <unordered_map>

#include "core/FileWatcher.hpp"
#include "core/Types.hpp"
#include "core/UniquelyIdentifiable.hpp"
#include "imgui_internal.h"
#include "ui/UI.hpp"
#include "util/BitCast.hpp"
#include "vulkan/Image.hpp"
#include "vulkan/Texture.hpp"

namespace Disarray::UI {

static inline auto rect_offset(const ImRect& rect, float x, float y)
{
	ImRect result = rect;
	result.Min.x += x;
	result.Min.y += y;
	result.Max.x += x;
	result.Max.y += y;
	return result;
}

template <std::int32_t Size, typename T> auto to_imgui(const glm::vec<Size, T>& vec) -> decltype(auto)
{
	if constexpr (Size == 2) {
		return ImVec2(vec[0], vec[1]);
	}
	if constexpr (Size == 3) {
		return ImVec3(vec[0], vec[1], vec[2]);
	}
	if constexpr (Size == 4) {
		return ImVec4(vec[0], vec[1], vec[2], vec[3]);
	}

	unreachable();
}

auto get_cache() -> auto& { return InterfaceCaches::descriptor_cache(); }

static auto add_image(VkSampler sampler, VkImageView view, VkImageLayout layout) -> ImageIdentifier
{
	auto* added = ImGui_ImplVulkan_AddTexture(sampler, view, layout);
	return Disarray::bit_cast<ImageIdentifier>(added);
}

static auto add_image(VkDescriptorImageInfo info) -> ImageIdentifier { return add_image(info.sampler, info.imageView, info.imageLayout); }

auto button(std::string_view label, const glm::vec2& size) -> bool { return ImGui::Button(label.data(), to_imgui(size)); }

auto image_button(const Disarray::Image& image, glm::vec2 size, const std::array<glm::vec2, 2>& uvs) -> bool
{
	const auto& vk_image = cast_to<Vulkan::Image>(image);

	const auto hash = vk_image.hash();
	auto& cache = get_cache();
	ImageIdentifier identifier = 0;
	if (!get_cache().contains(hash)) {
		identifier = add_image(vk_image.get_descriptor_info());
		cache.try_emplace(hash, std::make_unique<ImageIdentifier>(identifier));
	} else {
		identifier = *get_cache()[hash];
	}

	return ImGui::ImageButton(vk_image.get_properties().debug_name.c_str(), identifier, to_imgui<2>(size), to_imgui<2>(uvs[0]), to_imgui<2>(uvs[1]));
}

void image(const Disarray::Image& image, glm::vec2 size, const std::array<glm::vec2, 2>& uvs)
{
	const auto& vk_image = cast_to<Vulkan::Image>(image);

	const auto hash = vk_image.hash();
	auto& cache = get_cache();
	ImageIdentifier identifier = 0;
	if (!cache.contains(hash)) {
		identifier = add_image(vk_image.get_descriptor_info());
		cache.try_emplace(hash, std::make_unique<ImageIdentifier>(identifier));
	} else {
		identifier = *cache[hash];
	}

	ImGui::Image(identifier, to_imgui<2>(size), to_imgui<2>(uvs[0]), to_imgui<2>(uvs[1]));
}

auto image_button(const Disarray::Texture& tex, glm::vec2 size, const std::array<glm::vec2, 2>& uvs) -> bool
{
	const auto& vk_image = cast_to<Vulkan::Image>(tex.get_image());
	return image_button(vk_image, size, uvs);
}

void image(const Disarray::Texture& tex, glm::vec2 size, const std::array<glm::vec2, 2>& uvs)
{
	const auto& vk_image = cast_to<Vulkan::Image>(tex.get_image());
	image(vk_image, size, uvs);
}

void text(const std::string& formatted) { ImGui::Text("%s", formatted.c_str()); }
void text_wrapped(const std::string& formatted) { ImGui::TextWrapped("%s", formatted.c_str()); }

void scope(std::string_view name, UIFunction&& func)
{
	ImGui::Begin(name.data(), nullptr);
	std::move(func)();
	ImGui::End();
}

auto is_mouse_double_clicked(MouseCode code) -> bool
{
	if (code == MouseCode::Left) {
		return ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
	}
	if (code == MouseCode::Right) {
		return ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Right);
	}
	return false;
}

auto is_item_hovered() -> bool { return ImGui::IsItemHovered(); }

void begin(std::string_view name) { ImGui::Begin(name.data(), nullptr); }

void end() { ImGui::End(); }

void shift_cursor_y(float by) { ImGui::SetCursorPosY(ImGui::GetCursorPosY() + by); }

auto begin_combo(std::string_view name, std::string_view data) -> bool { return ImGui::BeginCombo(name.data(), data.data()); }

void end_combo() { ImGui::EndCombo(); }

auto is_selectable(std::string_view name, const bool is_selected) -> bool { return ImGui::Selectable(name.data(), is_selected); }

void set_item_default_focus() { ImGui::SetItemDefaultFocus(); }

auto is_maximised(Window& window) -> bool
{
	auto* glfw_window = static_cast<GLFWwindow*>(window.native());
	return static_cast<bool>(glfwGetWindowAttrib(glfw_window, GLFW_MAXIMIZED));
}

auto checkbox(const std::string& name, bool& value) -> bool { return ImGui::Checkbox(name.c_str(), &value); }

auto begin_menu_bar(const ImRect& rect) -> bool
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	IM_ASSERT(!window->DC.MenuBarAppending);
	ImGui::BeginGroup(); // Backup position on layer 0 // FIXME: Misleading to use a group for that backup/restore
	ImGui::PushID("##menubar");

	const ImVec2 padding = window->WindowPadding;

	// We don't clip with current window clipping rectangle as it is already set to the area below. However we clip with window full rect.
	// We remove 1 worth of rounding to Max.x to that text in long menus and small windows don't tend to display over the lower-right rounded area,
	// which looks particularly glitchy.
	ImRect bar_rect = rect_offset(rect, 0.0f, padding.y); // window->MenuBarRect();
	ImRect clip_rect(IM_ROUND(ImMax(window->Pos.x, bar_rect.Min.x + window->WindowBorderSize + window->Pos.x - 10.0f)),
		IM_ROUND(bar_rect.Min.y + window->WindowBorderSize + window->Pos.y),
		IM_ROUND(ImMax(bar_rect.Min.x + window->Pos.x, bar_rect.Max.x - ImMax(window->WindowRounding, window->WindowBorderSize))),
		IM_ROUND(bar_rect.Max.y + window->Pos.y));

	clip_rect.ClipWith(window->OuterRectClipped);
	ImGui::PushClipRect(clip_rect.Min, clip_rect.Max, false);

	// We overwrite CursorMaxPos because BeginGroup sets it to CursorPos (essentially the .EmitItem hack in EndMenuBar() would need something
	// analogous here, maybe a BeginGroupEx() with flags).
	window->DC.CursorPos = window->DC.CursorMaxPos = ImVec2(bar_rect.Min.x + window->Pos.x, bar_rect.Min.y + window->Pos.y);
	window->DC.LayoutType = ImGuiLayoutType_Horizontal;
	window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;
	window->DC.MenuBarAppending = true;
	ImGui::AlignTextToFramePadding();
	return true;
}

auto end_menu_bar() -> void
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return;
	ImGuiContext& g = *GImGui;

	// Nav: When a move request within one of our child menu failed, capture the request to navigate among our siblings.
	if (ImGui::NavMoveRequestButNoResultYet() && (g.NavMoveDir == ImGuiDir_Left || g.NavMoveDir == ImGuiDir_Right)
		&& (g.NavWindow->Flags & ImGuiWindowFlags_ChildMenu)) {
		// Try to find out if the request is for one of our child menu
		ImGuiWindow* nav_earliest_child = g.NavWindow;
		while (nav_earliest_child->ParentWindow && (nav_earliest_child->ParentWindow->Flags & ImGuiWindowFlags_ChildMenu))
			nav_earliest_child = nav_earliest_child->ParentWindow;
		if (nav_earliest_child->ParentWindow == window && nav_earliest_child->DC.ParentLayoutType == ImGuiLayoutType_Horizontal
			&& (g.NavMoveFlags & ImGuiNavMoveFlags_Forwarded) == 0) {
			// To do so we claim focus back, restore NavId and then process the movement request for yet another frame.
			// This involve a one-frame delay which isn't very problematic in this situation. We could remove it by scoring in advance for multiple
			// window (probably not worth bothering)
			const ImGuiNavLayer layer = ImGuiNavLayer_Menu;
			IM_ASSERT(window->DC.NavLayersActiveMaskNext & (1 << layer)); // Sanity check
			ImGui::FocusWindow(window);
			ImGui::SetNavID(window->NavLastIds[layer], layer, 0, window->NavRectRel[layer]);
			g.NavDisableHighlight = true; // Hide highlight for the current frame so we don't see the intermediary selection.
			g.NavDisableMouseHover = g.NavMousePosDirty = true;
			ImGui::NavMoveRequestForward(g.NavMoveDir, g.NavMoveClipDir, g.NavMoveFlags, g.NavMoveScrollFlags); // Repeat
		}
	}

	IM_MSVC_WARNING_SUPPRESS(6011); // Static Analysis false positive "warning C6011: Dereferencing NULL pointer 'window'"
	// IM_ASSERT(window->Flags & ImGuiWindowFlags_MenuBar); // NOTE(Yan): Needs to be commented out because Jay
	IM_ASSERT(window->DC.MenuBarAppending);
	ImGui::PopClipRect();
	ImGui::PopID();
	window->DC.MenuBarOffset.x = window->DC.CursorPos.x
		- window->Pos.x; // Save horizontal position so next append can reuse it. This is kinda equivalent to a per-layer CursorPos.
	g.GroupStack.back().EmitItem = false;
	ImGui::EndGroup(); // Restore position on layer 0
	window->DC.LayoutType = ImGuiLayoutType_Vertical;
	window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
	window->DC.MenuBarAppending = false;
}

auto shader_drop_button(Device& device, const std::string& button_name, ShaderType shader_type, Ref<Shader>& out_shader) -> bool
{
	UI::text_wrapped("Current shader: {}", out_shader->get_properties().identifier);
	ImGui::Button(button_name.c_str());
	if (const auto dropped = UI::accept_drag_drop("Disarray::DragDropItem", { ".vert", ".frag" })) {
		const auto& shader_path = *dropped;
		if (shader_path.extension() == shader_type_extension(shader_type)) {
			auto shader = Shader::compile(device, shader_path);
			out_shader = shader;
			return true;
		}
	}

	return false;
}

auto texture_drop_button(Device& device, const Texture& out_texture) -> Ref<Disarray::Texture>
{
	UI::image_button(out_texture.get_image());
	if (const auto dropped = UI::accept_drag_drop("Disarray::DragDropItem", { ".png", ".jpg", ".jpeg", ".ktx" })) {
		const std::filesystem::path& texture_path = *dropped;
		return Texture::construct(device,
			{
				.path = texture_path,
				.dimension = texture_path.extension().string() == ".ktx" ? TextureDimension::Three : TextureDimension::Two,
				.debug_name = texture_path.string(),
			});
	}

	return nullptr;
}

auto shader_drop_button(const Device& device, const std::string& button_name, ShaderType shader_type, Ref<Shader>& out_shader) -> bool
{
	UI::text_wrapped("Current shader: {}", out_shader->get_properties().identifier);
	ImGui::Button(button_name.c_str());
	if (const auto dropped = UI::accept_drag_drop("Disarray::DragDropItem", { ".vert", ".frag" })) {
		const auto& shader_path = *dropped;
		if (shader_path.extension() == shader_type_extension(shader_type)) {
			auto shader = Shader::compile(device, shader_path);
			out_shader = shader;
			return true;
		}
	}

	return false;
}

auto texture_drop_button(const Device& device, const Texture& out_texture) -> Ref<Disarray::Texture>
{
	UI::image_button(out_texture.get_image());
	if (const auto dropped = UI::accept_drag_drop("Disarray::DragDropItem", { ".png", ".jpg", ".jpeg" })) {
		const auto& texture_path = *dropped;
		return Texture::construct(device,
			{
				.path = std::filesystem::path(texture_path),
				.debug_name = texture_path.string(),
			});
	}

	return nullptr;
}

namespace Tabular {

	auto table(std::string_view name, const Collections::StringViewMap<std::string>& map) -> bool
	{
		if (ImGui::BeginTable(name.data(), 2)) {
			for (const auto& [key, value] : map) {
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				UI::text("{}", key);
				ImGui::TableNextColumn();
				UI::text("{}", value);
			}
			ImGui::EndTable();
		}
		return false;
	}

} // namespace Tabular

namespace Popup {

	auto select_file(const ExtensionSet& allowed, const std::filesystem::path& base) -> std::optional<std::filesystem::path>
	{
		// Create a file open dialog object.
		auto absolute = std::filesystem::absolute(base);
		std::vector<const char*> extensions {};
		extensions.reserve(allowed.size());
		for (const auto& ext : allowed) {
			extensions.push_back(ext.c_str());
		}
		const char* str = tinyfd_openFileDialog(
			"File selector", absolute.string().c_str(), static_cast<std::int32_t>(extensions.size()), extensions.data(), "", 0);

		if (str != nullptr) {
			return { std::filesystem::path { str } };
		}

		return std::nullopt;
	}

} // namespace Popup

namespace Input {

	auto general_slider(std::string_view name, int count, float* base, float min, float max) -> bool
	{
		return ImGui::SliderScalarN(name.data(), ImGuiDataType_Float, base, static_cast<int>(count), &min, &max);
	}

	auto general_drag(std::string_view name, int count, float* base, float velocity, float min, float max) -> bool
	{
		return ImGui::DragScalarN(name.data(), ImGuiDataType_Float, base, static_cast<int>(count), velocity, &min, &max);
	}

	auto general_input(std::string_view name, int count, float* base, float velocity, float min, float max) -> bool
	{
		return ImGui::InputScalarN(name.data(), ImGuiDataType_Float, base, static_cast<int>(count), &min, &max);
	}

	auto general_input(std::string_view name, int count, std::uint32_t* base, std::uint32_t min, std::uint32_t max) -> bool
	{
		return ImGui::InputScalarN(name.data(), ImGuiDataType_U32, base, static_cast<int>(count), &min, &max);
	}

	auto general_input(std::string_view name, int count, std::uint64_t* base, std::uint64_t min, std::uint64_t max) -> bool
	{
		return ImGui::InputScalarN(name.data(), ImGuiDataType_U64, base, static_cast<int>(count), &min, &max);
	}

	auto general_input(std::string_view name, int count, std::int32_t* base, std::int32_t min, std::int32_t max) -> bool
	{
		return ImGui::InputScalarN(name.data(), ImGuiDataType_S32, base, static_cast<int>(count), &min, &max);
	}

	auto general_input(std::string_view name, int count, std::int64_t* base, std::int64_t min, std::int64_t max) -> bool
	{
		return ImGui::InputScalarN(name.data(), ImGuiDataType_S64, base, static_cast<int>(count), &min, &max);
	}

} // namespace Input

void remove_image(const Texture& tex)
{
	const auto hash = tex.get_image().hash();
	remove_image(hash);
}

void remove_image(ImageIdentifier hash)
{
	auto& cache = get_cache();
	if (!cache.contains(hash)) {
		return;
	}
	const auto& value = cache[hash];
	ImGui_ImplVulkan_RemoveTexture(Disarray::bit_cast<VkDescriptorSet>(*value));
	cache.erase(hash);
}

void InterfaceCaches::initialise() { }

void InterfaceCaches::destruct()
{
	for (auto& [k, v] : image_descriptor_cache) {
		ImGui_ImplVulkan_RemoveTexture(Disarray::bit_cast<VkDescriptorSet>(*v));
	}
}

Scope::Scope(std::string_view name) { ImGui::Begin(name.data(), nullptr); }

Scope::~Scope() { ImGui::End(); }

} // namespace Disarray::UI
