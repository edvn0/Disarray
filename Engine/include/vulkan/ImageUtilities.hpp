#pragma once

#include <vulkan/vulkan.h>

namespace Disarray::Vulkan::Utilities {

	void insert_image_memory_barrier(VkCommandBuffer command_buffer, VkImage image, VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask,
		VkImageLayout old_image_layout, VkImageLayout new_image_layout, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
		VkImageSubresourceRange subresource_range);

	void set_image_layout(VkCommandBuffer command_buffer, VkImage image, VkImageLayout old_image_layout, VkImageLayout new_image_layout,
		VkImageSubresourceRange subresource_range, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask);
} // namespace Disarray::Vulkan::Utilities
