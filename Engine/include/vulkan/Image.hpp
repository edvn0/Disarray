#pragma once

#include "graphics/Image.hpp"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace Disarray::Vulkan {

	struct ImageInfo {
		VkImage image { nullptr };
		VkImageView view { nullptr };
		VkSampler sampler { nullptr };
		VmaAllocation allocation { nullptr };
	};

	static constexpr auto to_vulkan_format(ImageFormat format)
	{
		switch (format) {
		case ImageFormat::SRGB:
			return VK_FORMAT_R8G8B8A8_SRGB;
		case ImageFormat::RGB:
			return VK_FORMAT_R8G8B8_SRGB;
		case ImageFormat::SBGR:
			return VK_FORMAT_B8G8R8A8_SRGB;
		case ImageFormat::BGR:
			return VK_FORMAT_B8G8R8_SRGB;
		case ImageFormat::Depth:
			return VK_FORMAT_D32_SFLOAT;
		case ImageFormat::DepthStencil:
			return VK_FORMAT_D32_SFLOAT_S8_UINT;
		default:
			unreachable();
		}
	}

	class Image : public Disarray::Image {
	public:
		Image(Ref<Device>, Ref<Swapchain>, Ref<PhysicalDevice>, const ImageProperties&);
		~Image() override;

		void force_recreation() override { recreate(true); };
		void recreate(bool should_clean) override;

		VkImageView get_view() { return info.view; }
		VkImage get_image() { return info.image; }
		VkSampler get_sampler() { return info.sampler; }

	private:
		void recreate_image(bool should_clean);
		void update_descriptor();

		ImageInfo info {};
		VkDescriptorImageInfo descriptor_info;

		Ref<Device> device;
		Ref<Swapchain> swapchain;
		Ref<PhysicalDevice> physical_device;
		ImageProperties props;
	};

} // namespace Disarray::Vulkan
