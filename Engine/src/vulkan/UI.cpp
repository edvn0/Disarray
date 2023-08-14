#include "DisarrayPCH.hpp"

#include "Forward.hpp"
#include "backends/imgui_impl_vulkan.h"
#include "core/Types.hpp"
#include "core/UniquelyIdentifiable.hpp"
#include "ui/UI.hpp"
#include "util/BitCast.hpp"
#include "vulkan/Image.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <unordered_map>

namespace Disarray::UI {

template <std::size_t Size, typename T> decltype(auto) to_imgui(glm::vec<Size, T> vec)
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

auto& get_cache() { return DescriptorCache::get_cache(); }

static ImageIdentifier add_image(VkSampler sampler, VkImageView view, VkImageLayout layout)
{
	auto added = ImGui_ImplVulkan_AddTexture(sampler, view, layout);
	return Disarray::bit_cast<ImageIdentifier>(added);
}

static ImageIdentifier add_image(VkDescriptorImageInfo info) { return add_image(info.sampler, info.imageView, info.imageLayout); }

void image_button(Image& image, glm::vec2 size, const std::array<glm::vec2, 2>& uvs)
{
	auto& vk_image = cast_to<Vulkan::Image>(image);

	const auto hash = vk_image.hash();
	auto& cache = get_cache();
	ImageIdentifier id;
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
	ImageIdentifier id;
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

void scope(std::string_view name, UIFunction&& func)
{
	ImGui::Begin(name.data(), nullptr);
	func();
	ImGui::End();
}

bool is_mouse_double_clicked(MouseCode code)
{
	if (code == MouseCode::Left) {
		return ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
	}
	if (code == MouseCode::Right) {
		return ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Right);
	}
	return false;
}

bool is_item_hovered() { return ImGui::IsItemHovered(); }

void begin(std::string_view name) { ImGui::Begin(name.data(), nullptr); }

void end() { ImGui::End(); }

bool begin_combo(std::string_view name, std::string_view data) { return ImGui::BeginCombo(name.data(), data.data()); }
void end_combo() { ImGui::EndCombo(); }

bool is_selectable(std::string_view name, const bool is_selected) { return ImGui::Selectable(name.data(), is_selected); }

void set_item_default_focus() { ImGui::SetItemDefaultFocus(); }

bool is_maximised(Window& window)
{
	auto* glfw_window = static_cast<GLFWwindow*>(window.native());
	return static_cast<bool>(glfwGetWindowAttrib(glfw_window, GLFW_MAXIMIZED));
}

bool shader_drop_button(Device& device, const std::string& button_name, ShaderType shader_type, Ref<Shader>& out_shader)
{
	ImGui::Button(button_name.c_str());
	if (const auto dropped = UI::accept_drag_drop("Disarray::DragDropItem", ".spv")) {
		// We know that it is a spv file :)
		auto shader_path = *dropped;
		const auto ext = shader_path.replace_extension();
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

void DescriptorCache::initialise() { cache.reserve(100); }

void DescriptorCache::destruct()
{
	for (auto& [k, v] : cache) {
		ImGui_ImplVulkan_RemoveTexture(Disarray::bit_cast<VkDescriptorSet>(*v));
	}
}

} // namespace Disarray::UI
