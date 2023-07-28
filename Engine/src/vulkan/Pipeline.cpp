#pragma once

#include "vulkan/Pipeline.hpp"

#include "core/Types.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/Shader.hpp"
#include "vulkan/Swapchain.hpp"
#include "vulkan/vulkan_core.h"

namespace Disarray::Vulkan {

	VkPolygonMode vk_polygon_mode(PolygonMode mode)
	{
		switch (mode) {
		case PolygonMode::Fill:
			return VK_POLYGON_MODE_FILL;
		case PolygonMode::Line:
			return VK_POLYGON_MODE_LINE;
		case PolygonMode::Point:
			return VK_POLYGON_MODE_POINT;
		default:
			unreachable();
		}
	}

	Pipeline::Pipeline(Ref<Disarray::Device> dev,Ref<Disarray::Swapchain> sc, const Disarray::PipelineProperties& properties)
		: device(dev),
		swapchain(sc)
		, props(properties)
	{
		recreate(false);
	}

	void Pipeline::construct_layout()
	{
		std::vector<VkDynamicState> dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamic_state {};
		dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
		dynamic_state.pDynamicStates = dynamic_states.data();

		VkPipelineVertexInputStateCreateInfo vertex_input_info {};
		vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_info.vertexBindingDescriptionCount = 0;
		vertex_input_info.pVertexBindingDescriptions = nullptr; // Optional
		vertex_input_info.vertexAttributeDescriptionCount = 0;
		vertex_input_info.pVertexAttributeDescriptions = nullptr; // Optional

		VkPipelineInputAssemblyStateCreateInfo input_assembly {};
		input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = props.extent.width;
		viewport.height = props.extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor {};
		scissor.offset = { 0, 0 };
		scissor.extent
			= VkExtent2D { .width = static_cast<std::uint32_t>(props.extent.width), .height = static_cast<std::uint32_t>(props.extent.height) };

		VkPipelineViewportStateCreateInfo viewport_state {};
		viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.viewportCount = 1;
		viewport_state.pViewports = &viewport;
		viewport_state.scissorCount = 1;
		viewport_state.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = vk_polygon_mode(props.polygon_mode);

		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		VkPipelineMultisampleStateCreateInfo multisampling {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		VkPipelineColorBlendAttachmentState color_blend_attachment {};
		color_blend_attachment.colorWriteMask
			= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		color_blend_attachment.blendEnable = VK_FALSE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo color_blending {};
		color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blending.logicOpEnable = VK_FALSE;
		color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
		color_blending.attachmentCount = 1;
		color_blending.pAttachments = &color_blend_attachment;
		color_blending.blendConstants[0] = 0.0f; // Optional
		color_blending.blendConstants[1] = 0.0f; // Optional
		color_blending.blendConstants[2] = 0.0f; // Optional
		color_blending.blendConstants[3] = 0.0f; // Optional

		VkPipelineLayoutCreateInfo pipeline_layout_info {};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = 0; // Optional
		pipeline_layout_info.pSetLayouts = nullptr; // Optional
		pipeline_layout_info.pushConstantRangeCount = 0; // Optional
		pipeline_layout_info.pPushConstantRanges = nullptr; // Optional

		verify(vkCreatePipelineLayout(supply_cast<Vulkan::Device>(device), &pipeline_layout_info, nullptr, &layout));

		VkGraphicsPipelineCreateInfo pipeline_create_info {};
		pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_create_info.stageCount = 2;
		auto stages = retrieve_shader_stages(props.vertex_shader, props.fragment_shader);
		std::vector<VkPipelineShaderStageCreateInfo> stage_data { stages.first, stages.second };
		pipeline_create_info.pStages = stage_data.data();
		pipeline_create_info.pVertexInputState = &vertex_input_info;
		pipeline_create_info.pInputAssemblyState = &input_assembly;
		pipeline_create_info.pViewportState = &viewport_state;
		pipeline_create_info.pRasterizationState = &rasterizer;
		pipeline_create_info.pMultisampleState = &multisampling;
		pipeline_create_info.pDepthStencilState = nullptr; // Optional
		pipeline_create_info.pColorBlendState = &color_blending;
		pipeline_create_info.pDynamicState = &dynamic_state;
		pipeline_create_info.layout = layout;
		pipeline_create_info.renderPass = supply_cast<Vulkan::RenderPass>(props.render_pass);
		pipeline_create_info.subpass = 0;
		pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipeline_create_info.basePipelineIndex = -1; // Optional

		verify(vkCreateGraphicsPipelines(supply_cast<Vulkan::Device>(device), nullptr, 1, &pipeline_create_info, nullptr, &pipeline));
	}

	Pipeline::~Pipeline()
	{
		vkDestroyPipelineLayout(supply_cast<Vulkan::Device>(device), layout, nullptr);
		vkDestroyPipeline(supply_cast<Vulkan::Device>(device), pipeline, nullptr);

		props.vertex_shader->destroy_module();
		props.fragment_shader->destroy_module();
	}

	std::pair<VkPipelineShaderStageCreateInfo, VkPipelineShaderStageCreateInfo> Pipeline::retrieve_shader_stages(Ref<Disarray::Shader> vertex, Ref<Disarray::Shader> fragment) const
	{
		return { supply_cast<Vulkan::Shader>(vertex), supply_cast<Vulkan::Shader>(fragment) };
	}

	void Pipeline::recreate(bool should_clean) {
		if (should_clean) {
			vkDestroyPipelineLayout(supply_cast<Vulkan::Device>(device), layout, nullptr);
			vkDestroyPipeline(supply_cast<Vulkan::Device>(device), pipeline, nullptr);
		}

		props.extent = swapchain->get_extent();
		construct_layout();
	}

} // namespace Disarray::Vulkan