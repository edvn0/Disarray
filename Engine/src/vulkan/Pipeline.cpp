#include "DisarrayPCH.hpp"

#include "vulkan/Pipeline.hpp"

#include "core/Types.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PushConstantLayout.hpp"
#include "graphics/RenderPass.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/Shader.hpp"
#include "vulkan/Swapchain.hpp"
#include "vulkan/vulkan_core.h"

namespace Disarray::Vulkan {

	static constexpr VkFormat to_vulkan_format(ElementType type)
	{
		switch (type) {
		case ElementType::Float:
			return VK_FORMAT_R32_SFLOAT;
		case ElementType::Float2:
			return VK_FORMAT_R32G32_SFLOAT;
		case ElementType::Float3:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case ElementType::Float4:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case ElementType::Double:
			return VK_FORMAT_R64_SFLOAT;
		case ElementType::Int2:
			return VK_FORMAT_R16G16_SINT;
		case ElementType::Int3:
			return VK_FORMAT_R16G16B16_SINT;
		case ElementType::Int4:
			return VK_FORMAT_R16G16B16A16_SINT;
		case ElementType::Uint2:
			return VK_FORMAT_R16G16_UINT;
		case ElementType::Uint3:
			return VK_FORMAT_R16G16B16_UINT;
		case ElementType::Uint4:
			return VK_FORMAT_R16G16B16A16_UINT;
		default:
			unreachable();
		}
	}

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

	Pipeline::Pipeline(Disarray::Device& dev, Disarray::Swapchain& sc, const Disarray::PipelineProperties& properties)
		: device(dev)
		, swapchain(sc)
		, props(properties)
	{
		recreate_pipeline(false);
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

		const auto bindings = props.layout.construct_binding();
		VkVertexInputBindingDescription binding_description {};
		binding_description.binding = bindings.binding;
		binding_description.stride = bindings.stride;
		binding_description.inputRate = bindings.input_rate == InputRate::Vertex ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;

		std::vector<VkVertexInputAttributeDescription> attribute_descriptions {};
		std::uint32_t location = 0;
		for (const auto& attribute : props.layout.elements) {
			auto& new_attribute = attribute_descriptions.emplace_back();
			new_attribute.binding = 0;
			new_attribute.format = to_vulkan_format(attribute.type);
			new_attribute.offset = attribute.offset;
			new_attribute.location = location++;
		}

		vertex_input_info.vertexBindingDescriptionCount = 1;
		vertex_input_info.pVertexBindingDescriptions = &binding_description;
		vertex_input_info.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attribute_descriptions.size());
		vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

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

		rasterizer.lineWidth = props.line_width;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		depth_stencil_state_create_info.depthTestEnable = true;
		depth_stencil_state_create_info.depthWriteEnable = true;
		depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_state_create_info.back.compareOp = VK_COMPARE_OP_ALWAYS;
		depth_stencil_state_create_info.back.failOp = VK_STENCIL_OP_KEEP;
		depth_stencil_state_create_info.back.passOp = VK_STENCIL_OP_KEEP;
		depth_stencil_state_create_info.front = depth_stencil_state_create_info.back;
		depth_stencil_state_create_info.stencilTestEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = to_vulkan_samples(props.samples);
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		VkPipelineColorBlendAttachmentState color_blend_attachment {};
		color_blend_attachment.colorWriteMask
			= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		color_blend_attachment.blendEnable = VK_TRUE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo color_blending {};
		color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blending.logicOpEnable = VK_TRUE;
		color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
		color_blending.attachmentCount = 1;
		color_blending.pAttachments = &color_blend_attachment;
		color_blending.blendConstants[0] = 0.0f; // Optional
		color_blending.blendConstants[1] = 0.0f; // Optional
		color_blending.blendConstants[2] = 0.0f; // Optional
		color_blending.blendConstants[3] = 0.0f; // Optional

		VkPipelineLayoutCreateInfo pipeline_layout_info {};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = props.descriptor_set_layout_count; // Optional
		pipeline_layout_info.pSetLayouts = props.descriptor_set_layout; // Optional

		pipeline_layout_info.pushConstantRangeCount = static_cast<std::uint32_t>(props.push_constant_layout.size()); // Optional
		std::vector<VkPushConstantRange> result;
		for (const auto& pc_layout : props.push_constant_layout.get_input_ranges()) {
			auto& out = result.emplace_back();
			VkShaderStageFlags flags {};
			if (pc_layout.flags == PushConstantKind::Fragment) {
				flags = VK_SHADER_STAGE_FRAGMENT_BIT;
			}
			if (pc_layout.flags == PushConstantKind::Vertex) {
				flags = VK_SHADER_STAGE_VERTEX_BIT;
			}
			if (pc_layout.flags == PushConstantKind::Both) {
				flags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			}
			out = {
				.stageFlags = flags,
				.offset = pc_layout.offset,
				.size = pc_layout.size,
			};
		}
		pipeline_layout_info.pPushConstantRanges = result.data(); // Optional

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
		pipeline_create_info.pDepthStencilState = &depth_stencil_state_create_info; // Optional
		pipeline_create_info.pColorBlendState = &color_blending;
		pipeline_create_info.pDynamicState = &dynamic_state;
		pipeline_create_info.layout = layout;
		pipeline_create_info.renderPass = supply_cast<Vulkan::RenderPass>(props.framebuffer->get_render_pass());
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

	std::pair<VkPipelineShaderStageCreateInfo, VkPipelineShaderStageCreateInfo> Pipeline::retrieve_shader_stages(
		Ref<Disarray::Shader> vertex, Ref<Disarray::Shader> fragment) const
	{
		return { supply_cast<Vulkan::Shader>(*vertex), supply_cast<Vulkan::Shader>(*fragment) };
	}

	void Pipeline::recreate(bool should_clean) { recreate_pipeline(should_clean); }

	void Pipeline::recreate_pipeline(bool should_clean)
	{
		if (should_clean) {
			vkDestroyPipelineLayout(supply_cast<Vulkan::Device>(device), layout, nullptr);
			vkDestroyPipeline(supply_cast<Vulkan::Device>(device), pipeline, nullptr);
		}

		props.extent = swapchain.get_extent();
		construct_layout();
	}

	Disarray::Framebuffer& Pipeline::get_framebuffer() { return *props.framebuffer; }

	Disarray::RenderPass& Pipeline::get_render_pass() { return props.framebuffer->get_render_pass(); }

} // namespace Disarray::Vulkan
