#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace Disarray::Vulkan {

template <class T, std::size_t Count = 1> struct vk_structures {
	constexpr T operator()();
	constexpr std::array<T, Count> multiple()
	{
		std::array<T, Count> arr {};
		arr.fill(operator()());
		return arr;
	}
};

template <> struct vk_structures<VkSubmitInfo> {
	VkSubmitInfo operator()();
};

template <> struct vk_structures<VkDescriptorSetAllocateInfo> {
	VkDescriptorSetAllocateInfo operator()();
};

template <> struct vk_structures<VkPipelineColorBlendAttachmentState> {
	VkPipelineColorBlendAttachmentState operator()();
};

template <> struct vk_structures<VkAttachmentDescription> {
	VkAttachmentDescription operator()();
};

template <std::size_t Count> struct vk_structures<VkWriteDescriptorSet, Count> {
	VkWriteDescriptorSet operator()()
	{
		return {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
		};
	}
	std::array<VkWriteDescriptorSet, Count> multiple()
	{
		auto make = operator()();
		std::array<VkWriteDescriptorSet, Count> array;
		array.fill(make);
		return array;
	}
};

template <> struct vk_structures<VkDescriptorPoolSize> {
	VkDescriptorPoolSize operator()(std::uint32_t, VkDescriptorType);
};

template <> struct vk_structures<VkDescriptorSetLayoutBinding> {
	VkDescriptorSetLayoutBinding operator()(const VkSampler* = nullptr);
};

template <> struct vk_structures<VkDescriptorSetLayoutCreateInfo> {
	VkDescriptorSetLayoutCreateInfo operator()();
};

template <> struct vk_structures<VkShaderModuleCreateInfo> {
	VkShaderModuleCreateInfo operator()();
};

template <> struct vk_structures<VkPresentInfoKHR> {
	VkPresentInfoKHR operator()();
};

template <> struct vk_structures<VkSemaphoreCreateInfo> {
	VkSemaphoreCreateInfo operator()();
};

template <> struct vk_structures<VkFenceCreateInfo> {
	VkFenceCreateInfo operator()();
};

template <> struct vk_structures<VkSwapchainCreateInfoKHR> {
	VkSwapchainCreateInfoKHR operator()();
};

template <> struct vk_structures<VkCommandPoolCreateInfo> {
	VkCommandPoolCreateInfo operator()();
};

template <> struct vk_structures<VkCommandBufferAllocateInfo> {
	VkCommandBufferAllocateInfo operator()();
};

template <> struct vk_structures<VkRenderPassCreateInfo> {
	VkRenderPassCreateInfo operator()();
};

template <> struct vk_structures<VkRenderPassBeginInfo> {
	VkRenderPassBeginInfo operator()();
};

template <> struct vk_structures<VkPipelineDynamicStateCreateInfo> {
	VkPipelineDynamicStateCreateInfo operator()();
};

template <> struct vk_structures<VkPipelineVertexInputStateCreateInfo> {
	VkPipelineVertexInputStateCreateInfo operator()();
};

template <> struct vk_structures<VkPipelineInputAssemblyStateCreateInfo> {
	VkPipelineInputAssemblyStateCreateInfo operator()();
};

template <> struct vk_structures<VkPipelineViewportStateCreateInfo> {
	VkPipelineViewportStateCreateInfo operator()();
};

template <> struct vk_structures<VkPipelineRasterizationStateCreateInfo> {
	VkPipelineRasterizationStateCreateInfo operator()();
};

template <> struct vk_structures<VkPipelineDepthStencilStateCreateInfo> {
	VkPipelineDepthStencilStateCreateInfo operator()();
};

template <> struct vk_structures<VkPipelineMultisampleStateCreateInfo> {
	VkPipelineMultisampleStateCreateInfo operator()();
};

template <> struct vk_structures<VkPipelineColorBlendStateCreateInfo> {
	VkPipelineColorBlendStateCreateInfo operator()();
};

template <> struct vk_structures<VkPipelineLayoutCreateInfo> {
	VkPipelineLayoutCreateInfo operator()();
};

template <> struct vk_structures<VkGraphicsPipelineCreateInfo> {
	VkGraphicsPipelineCreateInfo operator()();
};

template <> struct vk_structures<VkComputePipelineCreateInfo> {
	VkGraphicsPipelineCreateInfo operator()();
};

template <> struct vk_structures<VkPipelineShaderStageCreateInfo> {
	VkPipelineShaderStageCreateInfo operator()();
};

template <> struct vk_structures<VkApplicationInfo> {
	VkApplicationInfo operator()();
};

template <> struct vk_structures<VkInstanceCreateInfo> {
	VkInstanceCreateInfo operator()();
};

template <> struct vk_structures<VkDebugUtilsMessengerCreateInfoEXT> {
	VkDebugUtilsMessengerCreateInfoEXT operator()();
};

template <> struct vk_structures<VkImageCreateInfo> {
	VkImageCreateInfo operator()();
};

template <> struct vk_structures<VkImageViewCreateInfo> {
	VkImageViewCreateInfo operator()();
};

template <> struct vk_structures<VkSamplerCreateInfo> {
	VkSamplerCreateInfo operator()();
};

template <> struct vk_structures<VkBufferCreateInfo> {
	VkBufferCreateInfo operator()();
};

template <> struct vk_structures<VkFramebufferCreateInfo> {
	VkFramebufferCreateInfo operator()();
};

template <> struct vk_structures<VkDeviceQueueCreateInfo> {
	VkDeviceQueueCreateInfo operator()();
};

template <> struct vk_structures<VkDeviceCreateInfo> {
	VkDeviceCreateInfo operator()();
};

template <> struct vk_structures<VkDebugMarkerObjectNameInfoEXT> {
	VkDebugMarkerObjectNameInfoEXT operator()();
};

template <> struct vk_structures<VkDebugMarkerObjectTagInfoEXT> {
	VkDebugMarkerObjectTagInfoEXT operator()();
};

template <> struct vk_structures<VkDebugMarkerMarkerInfoEXT> {
	VkDebugMarkerMarkerInfoEXT operator()();
};

template <> struct vk_structures<VkQueryPoolCreateInfo> {
	VkQueryPoolCreateInfo operator()();
};

template <> struct vk_structures<VkCommandBufferBeginInfo> {
	VkCommandBufferBeginInfo operator()();
};

template <> struct vk_structures<VkDescriptorPoolCreateInfo> {
	VkDescriptorPoolCreateInfo operator()();
};

template <> struct vk_structures<VkCommandBufferInheritanceInfo> {
	VkCommandBufferInheritanceInfo operator()();
};

} // namespace Disarray::Vulkan
