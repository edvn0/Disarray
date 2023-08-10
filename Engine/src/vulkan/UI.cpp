#include "DisarrayPCH.hpp"

#include "ui/UI.hpp"

#include "Forward.hpp"
#include "backends/imgui_impl_vulkan.h"
#include "core/Types.hpp"
#include "core/UniquelyIdentifiable.hpp"
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

	void image_button(Image& image, glm::vec2 size, const std::array<glm::vec2, 2>& uvs)
	{
		auto& vk_image = cast_to<Vulkan::Image>(image);

		const auto hash = vk_image.hash();
		auto& cache = get_cache();
		ImageIdentifier id;
		if (!get_cache().contains(hash)) {
			id = add_image(vk_image.get_sampler(), vk_image.get_view(), vk_image.get_layout());
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
			id = add_image(vk_image.get_sampler(), vk_image.get_view(), vk_image.get_layout());
			cache.try_emplace(hash, std::make_unique<ImageIdentifier>(id));
		} else {
			id = *cache[hash];
		}

		ImGui::Image(id, to_imgui<2>(size), to_imgui<2>(uvs[0]), to_imgui<2>(uvs[1]));
	}

	void scope(std::string_view name, UIFunction&& func)
	{
		ImGui::Begin(name.data(), nullptr);
		func();
		ImGui::End();
	}

	void begin(std::string_view name) { ImGui::Begin(name.data(), nullptr); }

	void end() { ImGui::End(); }

	bool is_maximised(Window& window)
	{
		auto* glfw_window = static_cast<GLFWwindow*>(window.native());
		return static_cast<bool>(glfwGetWindowAttrib(glfw_window, GLFW_MAXIMIZED));
	}

	void DescriptorCache::initialise() { cache.reserve(100); }

	void DescriptorCache::destruct()
	{
		for (auto& [k, v] : cache) {
			ImGui_ImplVulkan_RemoveTexture(Disarray::bit_cast<VkDescriptorSet>(*v));
		}
	}

} // namespace Disarray::UI
