#include "vulkan/Image.hpp"

#include "core/Types.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/ImageProperties.hpp"
#include "vulkan/Allocator.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/ImageUtilities.hpp"
#include "vulkan/vulkan_core.h"

namespace Disarray::Vulkan {

	static constexpr auto is_depth_format = [](auto format) {
		return format == ImageFormat::Depth || format == ImageFormat::DepthStencil;
	};

	Image::Image(Ref<Disarray::Device> dev, Ref<Disarray::Swapchain> sc, Ref<Disarray::PhysicalDevice> pd, const Disarray::ImageProperties& properties)
		: device(dev)
		, swapchain(sc)
		, physical_device(pd)
		, props(properties)
	{
		recreate_image(false);
	}

	Image::~Image()
	{
		vkDestroyImageView(supply_cast<Vulkan::Device>(device), info.view, nullptr);
		vkDestroySampler(supply_cast<Vulkan::Device>(device), info.sampler, nullptr);
		Allocator allocator { "Image Destructor" };
		allocator.deallocate_image(info.allocation, info.image);
		Log::debug("Destroyed image.");
	}

	void Image::recreate(bool should_clean)
	{
		recreate_image(should_clean);
	}

	void Image::recreate_image(bool should_clean)
	{
		if (should_clean) {
			Allocator allocator { "Image Recreator" };
			vkDestroyImageView(supply_cast<Vulkan::Device>(device), info.view, nullptr);
			vkDestroySampler(supply_cast<Vulkan::Device>(device), info.sampler, nullptr);
			allocator.deallocate_image(info.allocation, info.image);
		}

		if (is_depth_format(props.format)) {
			props.extent = swapchain->get_extent();
		}
		VkDeviceSize size = props.data.is_valid() ? props.data.get_size() : props.extent.get_size();

		VkBuffer staging;
		{
			VkBufferCreateInfo staging_create_info { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			staging_create_info.size = size;
			staging_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			staging_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			Allocator staging_allocator { "Image Recreator" };
			VmaAllocation staging_allocation = staging_allocator.allocate_buffer(
				staging, staging_create_info, { .usage = Usage::CPU_TO_GPU, .creation = Creation::HOST_ACCESS_RANDOM_BIT });
			{
				AllocationMapper<std::byte> mapper { staging_allocator, staging_allocation, props.data };
			}
			staging_allocator.deallocate_buffer(staging_allocation, staging);
		}


		VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT;
		if (is_depth_format(props.format)) {
				usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			} else {
				usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			}
		usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		VkImageAspectFlags aspect_mask = props.format == ImageFormat::Depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (props.format == ImageFormat::DepthStencil)
			aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;

		VkFormat vulkan_format = to_vulkan_format(props.format);

		VkImageCreateInfo image_create_info {};
		image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_create_info.imageType = VK_IMAGE_TYPE_2D;
		image_create_info.format = vulkan_format;
		image_create_info.extent.width = props.extent.width;
		image_create_info.extent.height = props.extent.height;
		image_create_info.extent.depth = 1;
		image_create_info.mipLevels = 1;
		image_create_info.arrayLayers = 1;
		image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_create_info.usage = usage;
		{
			Allocator allocator{"Image allocator"};
			info.allocation = allocator.allocate_image(info.image, image_create_info, {.usage = Usage::GPU_ONLY});
		}

		VkImageViewCreateInfo image_view_create_info {};
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format = vulkan_format;
		image_view_create_info.flags = 0;
		image_view_create_info.subresourceRange = {};
		image_view_create_info.subresourceRange.aspectMask = aspect_mask;
		image_view_create_info.subresourceRange.baseMipLevel = 0;
		image_view_create_info.subresourceRange.levelCount = 1;
		image_view_create_info.subresourceRange.baseArrayLayer = 0;
		image_view_create_info.subresourceRange.layerCount = 1;
		image_view_create_info.image = info.image;
		verify(vkCreateImageView(supply_cast<Vulkan::Device>(device), &image_view_create_info, nullptr, &info.view));

		VkSamplerCreateInfo sampler_create_info {};
		sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_create_info.maxAnisotropy = 1.0f;
		sampler_create_info.magFilter = VK_FILTER_LINEAR;
		sampler_create_info.minFilter = VK_FILTER_LINEAR;
		sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_create_info.addressModeV = sampler_create_info.addressModeU;
		sampler_create_info.addressModeW = sampler_create_info.addressModeU;
		sampler_create_info.mipLodBias = 0.0f;
		sampler_create_info.maxAnisotropy = 1.0f;
		sampler_create_info.minLod = 0.0f;
		sampler_create_info.maxLod = 100.0f;
		sampler_create_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		verify(vkCreateSampler(supply_cast<Vulkan::Device>(device), &sampler_create_info, nullptr, &info.sampler));

		update_descriptor();

		if (!props.data) return;

		auto&& [immediate, destructor] = construct_immediate<Vulkan::CommandExecutor>(device, swapchain, physical_device->get_queue_family_indexes());

		VkImageSubresourceRange subresource_range = {};
		subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresource_range.baseMipLevel = 0;
		subresource_range.levelCount = 1;
		subresource_range.layerCount = 1;

		VkImageMemoryBarrier image_memory_barrier {};
		image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		image_memory_barrier.image = info.image;
		image_memory_barrier.subresourceRange = subresource_range;
		image_memory_barrier.srcAccessMask = 0;
		image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		vkCmdPipelineBarrier(immediate->supply(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
			&image_memory_barrier);

		VkBufferImageCopy buffer_copy_region = {};
		buffer_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		buffer_copy_region.imageSubresource.mipLevel = 0;
		buffer_copy_region.imageSubresource.baseArrayLayer = 0;
		buffer_copy_region.imageSubresource.layerCount = 1;
		buffer_copy_region.imageExtent.width = props.extent.width;
		buffer_copy_region.imageExtent.height = props.extent.height;
		buffer_copy_region.imageExtent.depth = 1;
		buffer_copy_region.bufferOffset = 0;

		vkCmdCopyBufferToImage(
			immediate->supply(), staging, info.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_copy_region);

		Utilities::insert_image_memory_barrier(immediate->supply(), info.image, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, descriptor_info.imageLayout, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, subresource_range);

		destructor(immediate);
	}

	void Image::update_descriptor() {
		if (props.format == ImageFormat::DepthStencil || props.format == ImageFormat::Depth)
			descriptor_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		else
			descriptor_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		descriptor_info.imageView = info.view;
		descriptor_info.sampler = info.sampler;
	}

}