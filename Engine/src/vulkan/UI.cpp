#include "ui/UI.hpp"

#include "Forward.hpp"
#include "backends/imgui_impl_vulkan.h"
#include "core/Types.hpp"
#include "core/UniquelyIdentifiable.hpp"
#include "imgui.h"
#include "vulkan/Image.hpp"

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

	using ImageIdentifier = Identifier;

	static std::unordered_map<Identifier, ImageIdentifier> descriptor_info_cache;

	static ImageIdentifier add_image(VkSampler sampler, VkImageView view, VkImageLayout layout)
	{
		auto added = ImGui_ImplVulkan_AddTexture(sampler, view, layout);
		return reinterpret_cast<ImageIdentifier>(added);
	}

	void image_button(Image& image, glm::vec2 size)
	{
		auto& vk_image = cast_to<Vulkan::Image>(image);

		const auto hash = vk_image.hash();
		ImageIdentifier id;
		if (!descriptor_info_cache.contains(hash)) {
			id = add_image(vk_image.get_sampler(), vk_image.get_view(), vk_image.get_layout());
			descriptor_info_cache[hash] = id;
		} else {
			id = descriptor_info_cache[hash];
		}

		ImGui::ImageButton("Image", id, to_imgui<2>(size));
	}

	void image(Image& image, glm::vec2 size)
	{
		auto& vk_image = cast_to<Vulkan::Image>(image);

		const auto hash = vk_image.hash();
		ImageIdentifier id;
		if (!descriptor_info_cache.contains(hash)) {
			id = add_image(vk_image.get_sampler(), vk_image.get_view(), vk_image.get_layout());
			descriptor_info_cache[hash] = id;
		} else {
			id = descriptor_info_cache[hash];
		}

		ImGui::Image(id, to_imgui<2>(size));
	}

} // namespace Disarray::UI
