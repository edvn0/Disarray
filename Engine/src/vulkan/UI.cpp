#include "ui/UI.hpp"

#include "Forward.hpp"
#include "backends/imgui_impl_vulkan.h"
#include "core/Types.hpp"
#include "core/UniquelyIdentifiable.hpp"
#include "imgui.h"
#include "vulkan/Image.hpp"

#include <bit>
#include <memory>
#include <unordered_map>

namespace Disarray::UI {

	using ImageIdentifier = Identifier;

	static std::unordered_map<Identifier, ImageIdentifier> descriptor_info_cache;

	static ImageIdentifier add_image(VkSampler sampler, VkImageView view, VkImageLayout layout)
	{
		auto added = ImGui_ImplVulkan_AddTexture(sampler, view, layout);
		return reinterpret_cast<ImageIdentifier>(added);
	}

	void image_button(Image& image)
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

		ImGui::ImageButton("Image", id, { 64, 64 });
	}

} // namespace Disarray::UI
