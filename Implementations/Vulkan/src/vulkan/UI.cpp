#include "DisarrayPCH.hpp"

#include "ui/UI.hpp"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_vulkan.h>
#include <vulkan/vulkan.h>

#include <imgui.h>
#include <tinyfiledialogs.h>

#include <filesystem>
#include <unordered_map>

#include "Forward.hpp"
#include "core/FileWatcher.hpp"
#include "core/Types.hpp"
#include "core/UniquelyIdentifiable.hpp"
#include "util/BitCast.hpp"
#include "vulkan/Image.hpp"
#include "vulkan/Texture.hpp"

namespace Disarray::UI {

template <std::size_t Size, typename T> auto to_imgui(glm::vec<Size, T> vec) -> decltype(auto)
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

void image_button(Image& image, glm::vec2 size, const std::array<glm::vec2, 2>& uvs)
{
	auto& vk_image = cast_to<Vulkan::Image>(image);

	const auto hash = vk_image.hash();
	auto& cache = get_cache();
	ImageIdentifier identifier = 0;
	if (!get_cache().contains(hash)) {
		identifier = add_image(vk_image.get_descriptor_info());
		cache.try_emplace(hash, std::make_unique<ImageIdentifier>(identifier));
	} else {
		identifier = *get_cache()[hash];
	}

	ImGui::ImageButton("Image", identifier, to_imgui<2>(size), to_imgui<2>(uvs[0]), to_imgui<2>(uvs[1]));
}

void image_button(const Image& image, glm::vec2 size, const std::array<glm::vec2, 2>& uvs)
{
	const auto& vk_image = cast_to<Vulkan::Image>(image);

	auto hash = vk_image.hash();
	auto& cache = get_cache();
	ImageIdentifier identifier = 0;
	if (!get_cache().contains(hash)) {
		identifier = add_image(vk_image.get_descriptor_info());
		cache.try_emplace(hash, std::make_unique<ImageIdentifier>(identifier));
	} else {
		identifier = *get_cache()[hash];
	}

	ImGui::ImageButton("Image", identifier, to_imgui<2>(size), to_imgui<2>(uvs[0]), to_imgui<2>(uvs[1]));
}

void image(Image& image, glm::vec2 size, const std::array<glm::vec2, 2>& uvs)
{
	auto& vk_image = cast_to<Vulkan::Image>(image);

	const auto hash = vk_image.hash();
	auto& cache = get_cache();
	ImageIdentifier identifier = 0;
	if (!get_cache().contains(hash)) {
		identifier = add_image(vk_image.get_descriptor_info());
		cache.try_emplace(hash, std::make_unique<ImageIdentifier>(identifier));
	} else {
		identifier = *cache[hash];
	}

	ImGui::Image(identifier, to_imgui<2>(size), to_imgui<2>(uvs[0]), to_imgui<2>(uvs[1]));
}

void image_button(Texture& tex, glm::vec2 size, const std::array<glm::vec2, 2>& uvs)
{
	auto& vk_image = cast_to<Vulkan::Image>(tex.get_image());
	image_button(vk_image, size, uvs);
}

void image(Texture& tex, glm::vec2 size, const std::array<glm::vec2, 2>& uvs)
{
	auto& vk_image = cast_to<Vulkan::Image>(tex.get_image());
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

namespace Tabular {

	auto table(std::string_view name, const Collections::StringViewMap<std::string>& map) -> bool
	{
		ImGui::Begin(name.data());
		if (!ImGui::BeginTable(name.data(), 2)) {
			ImGui::EndTable();
			ImGui::End();
			return false;
		}

		for (const auto& [key, value] : map) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			UI::text("{}", key);
			ImGui::TableNextColumn();
			UI::text("{}", value);
		}

		ImGui::EndTable();
		ImGui::End();
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
		const char* str = tinyfd_openFileDialog("File selector", absolute.string().c_str(), 1, extensions.data(), nullptr, 0);

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

} // namespace Input

auto texture_drop_button(Device& device, const Texture& out_texture) -> Ref<Disarray::Texture>
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

} // namespace Disarray::UI
