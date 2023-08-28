#include "DisarrayPCH.hpp"

#include "graphics/Image.hpp"

#include <vulkan/vulkan.h>

#include <span>

#include "core/Ensure.hpp"
#include "core/Formatters.hpp"
#include "core/ThreadPool.hpp"
#include "core/Types.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/ImageProperties.hpp"
#include "util/FormattingUtilities.hpp"
#include "vulkan/Allocator.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Image.hpp"

namespace Disarray::Vulkan {
using Disarray::ImageProperties;

static void set_image_layout(VkCommandBuffer command_buffer, VkImage image, VkImageLayout old_image_layout, VkImageLayout new_image_layout,
	VkImageSubresourceRange subresource_range, VkPipelineStageFlags src_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
	VkPipelineStageFlags dst_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

Image::Image(const Disarray::Device& dev, ImageProperties properties)
	: Disarray::Image(std::move(properties))
	, device(dev)
{
	recreate_image(false);
}

Image::~Image() { destroy_resources(); }

void Image::destroy_resources()
{
	Allocator allocator { "Image[" + get_properties().debug_name + "]" };
	const auto& vk_device = supply_cast<Vulkan::Device>(device);
	vkDestroyImageView(vk_device, info.view, nullptr);
	vkDestroySampler(vk_device, info.sampler, nullptr);
	vkDestroyDescriptorSetLayout(vk_device, layout, nullptr);
	allocator.deallocate_image(info.allocation, info.image);
	info.view = nullptr;
	info.sampler = nullptr;
	info.image = nullptr;
	update_descriptor();
}

void Image::recreate(bool should_clean, const Extent& extent)
{
	Log::info("Image", "OLD SIZE!!! {}", get_properties().extent);
	get_properties().extent = extent;
	Log::info("Image", "SET SIZE!!! {}", get_properties().extent);
	recreate_image(should_clean);
}

auto Image::read_pixel(const glm::vec2& pos) const -> PixelReadData
{
	PixelReadData read_data { std::monostate {} };
	VkBuffer pixel_data_buffer { nullptr };
	Allocator allocator { "ReadPixelData" };
	VmaAllocation allocation {};

	ImageProperties copy = get_properties();
	copy.tiling = Tiling::Linear;
	auto staging_image = make_scope<Vulkan::Image>(device, copy);

	{
		auto immediate = construct_immediate(device);
		VkImageAspectFlags aspect_mask = get_properties().format == ImageFormat::Depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (get_properties().format == ImageFormat::DepthStencil) {
			aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		VkImageSubresourceRange subresource_range = {};
		subresource_range.layerCount = 1;
		subresource_range.aspectMask = aspect_mask;
		subresource_range.baseMipLevel = 0;
		subresource_range.levelCount = 1;
		set_image_layout(immediate->supply(), info.image, descriptor_info.imageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, subresource_range);
		set_image_layout(
			immediate->supply(), staging_image->supply(), descriptor_info.imageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresource_range);

		auto create_info = vk_structures<VkBufferCreateInfo>()();
		const auto& img_props = get_properties();
		const auto img_size = img_props.data.get_size();
		VkDeviceSize size = get_properties().data.is_valid() ? img_size : img_props.extent.get_size() * sizeof(float);
		create_info.size = size;
		create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		allocation = allocator.allocate_buffer(
			pixel_data_buffer, create_info, { .usage = Usage::AUTO_PREFER_HOST, .creation = Creation::HOST_ACCESS_RANDOM_BIT });

		VkBufferImageCopy buffer_copy_region = {};
		buffer_copy_region.imageSubresource.aspectMask = aspect_mask;
		buffer_copy_region.imageSubresource.mipLevel = 0;
		buffer_copy_region.imageSubresource.baseArrayLayer = 0;
		buffer_copy_region.imageSubresource.layerCount = 1;
		buffer_copy_region.imageExtent.width = get_properties().extent.width;
		buffer_copy_region.imageExtent.height = get_properties().extent.height;
		buffer_copy_region.imageExtent.depth = 1;
		buffer_copy_region.bufferOffset = 0;

		VkImageCopy copy_region {};
		VkImageSubresourceLayers layers = {};
		layers.layerCount = 1;
		layers.aspectMask = aspect_mask;
		layers.mipLevel = 0;

		copy_region.srcSubresource = layers;
		copy_region.dstSubresource = layers;
		copy_region.extent = { .width = get_properties().extent.width, .height = get_properties().extent.height, .depth = 1 };

		auto* vk_cmd = immediate->supply();

		vkCmdCopyImage(
			vk_cmd, info.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, staging_image->supply(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
		set_image_layout(immediate->supply(), info.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, descriptor_info.imageLayout, subresource_range);

		set_image_layout(
			vk_cmd, staging_image->supply(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, subresource_range);
		vkCmdCopyImageToBuffer(vk_cmd, staging_image->supply(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, pixel_data_buffer, 1, &buffer_copy_region);

		VkImageSubresource subresource {};
		subresource.aspectMask = aspect_mask;
		subresource.mipLevel = 0;
		VkSubresourceLayout subresource_layout {};
		vkGetImageSubresourceLayout(supply_cast<Vulkan::Device>(device), staging_image->supply(), &subresource, &subresource_layout);

		auto* data = allocator.map_memory<std::byte>(allocation);

		const auto to_float_extent = get_properties().extent.as<float>();
		auto offset_x = static_cast<std::uint32_t>(pos[0] * to_float_extent.width);
		auto offset_y = static_cast<std::uint32_t>(pos[1] * to_float_extent.height);
		std::size_t offset = (offset_y * get_properties().extent.width + offset_x) * sizeof(float);

		ensure(data != nullptr);

		auto span = std::span { data, size };
		if (get_properties().format == ImageFormat::Uint) {
			// ID of the entity
			read_data = static_cast<std::uint32_t>(span[offset]);
		} else {
			// Pixel colour
			read_data = glm::vec4 { span[offset], span[offset + 1], span[offset + 2], span[offset + 3] };
		}
		allocator.unmap_memory(allocation);
	}
	allocator.deallocate_buffer(allocation, pixel_data_buffer);
	return read_data;
}

void Image::recreate_image(bool should_clean)
{
	if (should_clean) {
		destroy_resources();
	}

	VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	if (is_depth_format(get_properties().format)) {
		usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	} else {
		usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	VkImageAspectFlags aspect_mask = get_properties().format == ImageFormat::Depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	if (get_properties().format == ImageFormat::DepthStencil) {
		aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	VkFormat vulkan_format = to_vulkan_format(get_properties().format);

	VkImageCreateInfo image_create_info {};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;
	image_create_info.format = vulkan_format;
	image_create_info.extent.width = get_properties().extent.width;
	image_create_info.extent.height = get_properties().extent.height;
	image_create_info.extent.depth = 1;
	image_create_info.samples = to_vulkan_samples(get_properties().samples);
	image_create_info.mipLevels = get_properties().mips;
	image_create_info.arrayLayers = 1;
	image_create_info.tiling = to_vulkan_tiling(get_properties().tiling);
	image_create_info.usage = usage;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	{
		Allocator allocator { "Image[" + get_properties().debug_name + "]" };
		info.allocation = allocator.allocate_image(info.image, image_create_info, { .usage = Usage::GPU_ONLY });
	}

	VkImageViewCreateInfo image_view_create_info {};
	image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_create_info.format = vulkan_format;
	image_view_create_info.flags = 0;
	image_view_create_info.subresourceRange = {};
	image_view_create_info.subresourceRange.aspectMask = aspect_mask;
	image_view_create_info.subresourceRange.baseMipLevel = 0;
	image_view_create_info.subresourceRange.levelCount = get_properties().mips;
	image_view_create_info.subresourceRange.baseArrayLayer = 0;
	image_view_create_info.subresourceRange.layerCount = 1;
	image_view_create_info.image = info.image;
	verify(vkCreateImageView(supply_cast<Vulkan::Device>(device), &image_view_create_info, nullptr, &info.view));

	VkSamplerCreateInfo sampler_create_info {};
	sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_create_info.maxAnisotropy = 1.0F;
	sampler_create_info.magFilter = VK_FILTER_LINEAR;
	sampler_create_info.minFilter = VK_FILTER_LINEAR;
	sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_create_info.addressModeV = sampler_create_info.addressModeU;
	sampler_create_info.addressModeW = sampler_create_info.addressModeU;
	sampler_create_info.mipLodBias = 0.0F;
	sampler_create_info.maxAnisotropy = 1.0F;
	sampler_create_info.minLod = 0.0F;
	sampler_create_info.maxLod = static_cast<float>(get_properties().mips);
	sampler_create_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	verify(vkCreateSampler(supply_cast<Vulkan::Device>(device), &sampler_create_info, nullptr, &info.sampler));

	update_descriptor();

	VkDeviceSize size { 0 };
	if (get_properties().data.is_valid()) {
		size = get_properties().data.get_size();
	} else {
		size = get_properties().extent.get_size() * sizeof(float);
		auto& buffer = get_properties().data;
		buffer.allocate(size);
	}
	VkBuffer staging {};
	VkBufferCreateInfo staging_create_info {};
	staging_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	staging_create_info.size = size;
	staging_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	staging_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	Allocator staging_allocator { "Image Recreator" };
	VmaAllocation staging_allocation = staging_allocator.allocate_buffer(
		staging, staging_create_info, { .usage = Usage::CPU_TO_GPU, .creation = Creation::HOST_ACCESS_RANDOM_BIT });
	{
		AllocationMapper<std::byte> mapper { staging_allocator, staging_allocation, get_properties().data };
	}

	{
		auto executor = construct_immediate(device);
		VkImageSubresourceRange subresource_range = {};
		subresource_range.aspectMask = aspect_mask;
		subresource_range.baseMipLevel = 0;
		subresource_range.levelCount = get_properties().mips;
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

		vkCmdPipelineBarrier(
			executor->supply(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);

		VkBufferImageCopy buffer_copy_region = {};
		buffer_copy_region.imageSubresource.aspectMask = aspect_mask;
		buffer_copy_region.imageSubresource.mipLevel = 0;
		buffer_copy_region.imageSubresource.baseArrayLayer = 0;
		buffer_copy_region.imageSubresource.layerCount = 1;
		buffer_copy_region.imageExtent.width = get_properties().extent.width;
		buffer_copy_region.imageExtent.height = get_properties().extent.height;
		buffer_copy_region.imageExtent.depth = 1;
		buffer_copy_region.bufferOffset = 0;

		set_image_layout(executor->supply(), info.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresource_range);
		vkCmdCopyBufferToImage(executor->supply(), staging, info.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_copy_region);

		auto image_layout = to_vulkan_layout(get_properties().format);
		if (is_depth_format(get_properties().format)) {
			set_image_layout(executor->supply(), info.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, image_layout, subresource_range);
		}
	}

	staging_allocator.deallocate_buffer(staging_allocation, staging);
	if (!is_depth_format(get_properties().format)) {
		create_mips();
	}

	std::array<VkDescriptorSetLayoutBinding, 1> bindings {};
	auto& image = bindings[0];
	image.binding = 0;
	image.descriptorCount = 1;
	image.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	image.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	auto layout_create_info = vk_structures<VkDescriptorSetLayoutCreateInfo>()();
	layout_create_info.bindingCount = 1;
	layout_create_info.pBindings = bindings.data();

	vkCreateDescriptorSetLayout(supply_cast<Vulkan::Device>(device), &layout_create_info, nullptr, &layout);
}

void Image::update_descriptor()
{
	descriptor_info.imageLayout = to_vulkan_layout(get_properties().format);
	descriptor_info.imageView = info.view;
	descriptor_info.sampler = info.sampler;
}

void set_image_layout(VkCommandBuffer command_buffer, VkImage image, VkImageLayout old_image_layout, VkImageLayout new_image_layout,
	VkImageSubresourceRange subresource_range, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask)
{
	VkImageMemoryBarrier image_memory_barrier = {};
	image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	image_memory_barrier.oldLayout = old_image_layout;
	image_memory_barrier.newLayout = new_image_layout;
	image_memory_barrier.image = image;
	image_memory_barrier.subresourceRange = subresource_range;

	switch (old_image_layout) {
	case VK_IMAGE_LAYOUT_UNDEFINED:
		image_memory_barrier.srcAccessMask = 0;
		break;

	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		image_memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		image_memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		image_memory_barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		image_memory_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		break;
	}

	switch (new_image_layout) {
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		image_memory_barrier.dstAccessMask = image_memory_barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		if (image_memory_barrier.srcAccessMask == 0) {
			image_memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		break;
	}

	vkCmdPipelineBarrier(command_buffer, src_stage_mask, dst_stage_mask, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
}

void Image::create_mips()
{
	VkImageMemoryBarrier barrier {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = info.image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;
	auto&& [mip_width, mip_height] = get_properties().extent.cast<int>();

	auto command_buffer = construct_immediate(device);
	for (std::uint32_t i = 1; i < get_properties().mips; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(
			command_buffer->supply(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		VkImageBlit blit {};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mip_width, mip_height, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(command_buffer->supply(), info.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, info.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit, VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(
			command_buffer->supply(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		if (mip_width > 1) {
			mip_width /= 2;
		}
		if (mip_height > 1) {
			mip_height /= 2;
		}
	}

	barrier.subresourceRange.baseMipLevel = get_properties().mips - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(
		command_buffer->supply(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

} // namespace Disarray::Vulkan
