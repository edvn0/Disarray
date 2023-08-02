#pragma once

#include "core/UniquelyIdentifiable.hpp"
#include "graphics/Image.hpp"
#include "vulkan/MemoryAllocator.hpp"
#include "vulkan/vulkan_core.h"

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

	class Image : public Disarray::Image, public Disarray::UniquelyIdentifiable<Vulkan::Image> {
	public:
		Image(Disarray::Device&, Disarray::Swapchain&, const ImageProperties&);
		~Image() override;

		void force_recreation() override { recreate(true); };
		void recreate(bool should_clean) override;

		VkImage get_image() const { return info.image; }
		VkImageView get_view() const { return descriptor_info.imageView; }
		VkSampler get_sampler() const { return descriptor_info.sampler; }
		VkImageLayout get_layout() const { return descriptor_info.imageLayout; }

		Identifier hash_impl() { return bit_cast<std::size_t>(descriptor_info.imageView) ^ bit_cast<std::size_t>(descriptor_info.sampler); };

	private:
		void recreate_image(bool should_clean);
		void update_descriptor();
		void destroy_resources();

		ImageInfo info {};
		VkDescriptorImageInfo descriptor_info;

		Device& device;
		Swapchain& swapchain;
		ImageProperties props;
	};

} // namespace Disarray::Vulkan
