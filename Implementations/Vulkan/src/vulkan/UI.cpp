#include "DisarrayPCH.hpp"

#include "ui/UI.hpp"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_vulkan.h>
#include <vulkan/vulkan.h>

#include <imgui.h>

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
	ImageIdentifier id = 0;
	if (!get_cache().contains(hash)) {
		id = add_image(vk_image.get_descriptor_info());
		cache.try_emplace(hash, std::make_unique<ImageIdentifier>(id));
	} else {
		id = *get_cache()[hash];
	}

	ImGui::ImageButton("Image", id, to_imgui<2>(size), to_imgui<2>(uvs[0]), to_imgui<2>(uvs[1]));
}

void image_button(const Image& image, glm::vec2 size, const std::array<glm::vec2, 2>& uvs)
{
	const auto& vk_image = cast_to<Vulkan::Image>(image);

	auto hash = vk_image.hash();
	auto& cache = get_cache();
	ImageIdentifier id = 0;
	if (!get_cache().contains(hash)) {
		id = add_image(vk_image.get_descriptor_info());
		cache.try_emplace(hash, std::make_unique<ImageIdentifier>(id));
	} else {
		id = *get_cache()[hash];
	}

	ImGui::ImageButton("Image", id, to_imgui<2>(size), to_imgui<2>(uvs[0]), to_imgui<2>(uvs[1]));
}

void image(Image& image, glm::vec2 size, const std::array<glm::vec2, 2>& uvs)
{
	auto& vk_image = cast_to<Vulkan::Image>(image);

	const auto hash = vk_image.hash();
	auto& cache = get_cache();
	ImageIdentifier id = 0;
	if (!get_cache().contains(hash)) {
		id = add_image(vk_image.get_descriptor_info());
		cache.try_emplace(hash, std::make_unique<ImageIdentifier>(id));
	} else {
		id = *cache[hash];
	}

	ImGui::Image(id, to_imgui<2>(size), to_imgui<2>(uvs[0]), to_imgui<2>(uvs[1]));
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
	ImGui::Button(button_name.c_str());
	if (const auto dropped = UI::accept_drag_drop("Disarray::DragDropItem", { ".spv" })) {
		// We know that it is a spv file :)
		auto shader_path = *dropped;
		auto ext = shader_path.replace_extension();
		if (ext.extension() == shader_type_extension(shader_type)) {
			auto shader = Shader::construct(device,
				ShaderProperties {
					.path = *dropped,
					.type = ShaderType::Vertex,
				});
			out_shader = shader;
			return true;
		}
	}

	return false;
}

auto texture_drop_button(Device& device, const Texture& out_texture) -> Ref<Texture>
{
	UI::image_button(out_texture.get_image());
	if (const auto dropped = UI::accept_drag_drop("Disarray::DragDropItem", { ".png", ".jpg", ".jpeg" })) {
		const auto& texture_path = *dropped;
		return Texture::construct(device, { .path = std::filesystem::path(texture_path) });
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

void InterfaceCaches::initialise() { image_descriptor_cache.reserve(100); }

void InterfaceCaches::destruct()
{
	for (auto& [k, v] : image_descriptor_cache) {
		ImGui_ImplVulkan_RemoveTexture(Disarray::bit_cast<VkDescriptorSet>(*v));
	}
}

} // namespace Disarray::UI
