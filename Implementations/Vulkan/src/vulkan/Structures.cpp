#include "DisarrayPCH.hpp"

#include "vulkan/Structures.hpp"

#include <vulkan/vulkan.h>

namespace Disarray::Vulkan {

VkSubmitInfo vk_structures<VkSubmitInfo>::operator()() { return { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .pNext = nullptr }; }
VkDescriptorSetAllocateInfo vk_structures<VkDescriptorSetAllocateInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, .pNext = nullptr };
}

VkPipelineColorBlendAttachmentState vk_structures<VkPipelineColorBlendAttachmentState>::operator()() { return {}; }

VkShaderModuleCreateInfo vk_structures<VkShaderModuleCreateInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, .pNext = nullptr };
}

VkAttachmentDescription vk_structures<VkAttachmentDescription>::operator()() { return {}; }

VkDescriptorPoolSize vk_structures<VkDescriptorPoolSize>::operator()(std::uint32_t count, VkDescriptorType type)
{
	return {
		.type = type,
		.descriptorCount = count,
	};
}

VkDescriptorSetLayoutBinding vk_structures<VkDescriptorSetLayoutBinding>::operator()(const VkSampler* samplers)
{
	return { .pImmutableSamplers = samplers };
}

VkDescriptorSetLayoutCreateInfo vk_structures<VkDescriptorSetLayoutCreateInfo>::operator()()
{
	return {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
	};
}

VkPresentInfoKHR vk_structures<VkPresentInfoKHR>::operator()() { return { .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, .pNext = nullptr }; }

VkSemaphoreCreateInfo vk_structures<VkSemaphoreCreateInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, .pNext = nullptr };
}

VkFenceCreateInfo vk_structures<VkFenceCreateInfo>::operator()() { return { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .pNext = nullptr }; }

VkSwapchainCreateInfoKHR vk_structures<VkSwapchainCreateInfoKHR>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, .pNext = nullptr };
}

VkCommandPoolCreateInfo vk_structures<VkCommandPoolCreateInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, .pNext = nullptr };
}

VkCommandBufferAllocateInfo vk_structures<VkCommandBufferAllocateInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, .pNext = nullptr };
}

VkRenderPassCreateInfo vk_structures<VkRenderPassCreateInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, .pNext = nullptr };
}

VkRenderPassBeginInfo vk_structures<VkRenderPassBeginInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, .pNext = nullptr };
}

VkPipelineDynamicStateCreateInfo vk_structures<VkPipelineDynamicStateCreateInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, .pNext = nullptr };
}

VkPipelineVertexInputStateCreateInfo vk_structures<VkPipelineVertexInputStateCreateInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, .pNext = nullptr };
}

VkPipelineInputAssemblyStateCreateInfo vk_structures<VkPipelineInputAssemblyStateCreateInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, .pNext = nullptr };
}

VkPipelineViewportStateCreateInfo vk_structures<VkPipelineViewportStateCreateInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .pNext = nullptr };
}

VkPipelineRasterizationStateCreateInfo vk_structures<VkPipelineRasterizationStateCreateInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, .pNext = nullptr };
}

VkPipelineDepthStencilStateCreateInfo vk_structures<VkPipelineDepthStencilStateCreateInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, .pNext = nullptr };
}

VkPipelineMultisampleStateCreateInfo vk_structures<VkPipelineMultisampleStateCreateInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, .pNext = nullptr };
}

VkPipelineColorBlendStateCreateInfo vk_structures<VkPipelineColorBlendStateCreateInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, .pNext = nullptr };
}

VkPipelineLayoutCreateInfo vk_structures<VkPipelineLayoutCreateInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, .pNext = nullptr };
}

VkGraphicsPipelineCreateInfo vk_structures<VkGraphicsPipelineCreateInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, .pNext = nullptr };
}

VkGraphicsPipelineCreateInfo vk_structures<VkComputePipelineCreateInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, .pNext = nullptr };
}

VkPipelineShaderStageCreateInfo vk_structures<VkPipelineShaderStageCreateInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr };
}

VkApplicationInfo vk_structures<VkApplicationInfo>::operator()() { return { .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .pNext = nullptr }; }

VkInstanceCreateInfo vk_structures<VkInstanceCreateInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, .pNext = nullptr };
}

VkDebugUtilsMessengerCreateInfoEXT vk_structures<VkDebugUtilsMessengerCreateInfoEXT>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT, .pNext = nullptr };
}

VkImageCreateInfo vk_structures<VkImageCreateInfo>::operator()() { return { .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, .pNext = nullptr }; }

VkImageViewCreateInfo vk_structures<VkImageViewCreateInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, .pNext = nullptr };
}

VkSamplerCreateInfo vk_structures<VkSamplerCreateInfo>::operator()() { return { .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, .pNext = nullptr }; }

VkBufferCreateInfo vk_structures<VkBufferCreateInfo>::operator()() { return { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .pNext = nullptr }; }

VkFramebufferCreateInfo vk_structures<VkFramebufferCreateInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, .pNext = nullptr };
}

VkDeviceQueueCreateInfo vk_structures<VkDeviceQueueCreateInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .pNext = nullptr };
}

VkDeviceCreateInfo vk_structures<VkDeviceCreateInfo>::operator()() { return { .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, .pNext = nullptr }; }

VkDebugMarkerObjectNameInfoEXT vk_structures<VkDebugMarkerObjectNameInfoEXT>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT, .pNext = nullptr };
}

VkDebugMarkerObjectTagInfoEXT vk_structures<VkDebugMarkerObjectTagInfoEXT>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT, .pNext = nullptr };
}

VkDebugMarkerMarkerInfoEXT vk_structures<VkDebugMarkerMarkerInfoEXT>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT, .pNext = nullptr };
}

VkQueryPoolCreateInfo vk_structures<VkQueryPoolCreateInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO, .pNext = nullptr };
}

VkCommandBufferBeginInfo vk_structures<VkCommandBufferBeginInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr };
}

VkDescriptorPoolCreateInfo vk_structures<VkDescriptorPoolCreateInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, .pNext = nullptr };
}

VkCommandBufferInheritanceInfo vk_structures<VkCommandBufferInheritanceInfo>::operator()()
{
	return { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO, .pNext = nullptr };
}

} // namespace Disarray::Vulkan
