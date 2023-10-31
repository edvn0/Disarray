#include "DisarrayPCH.hpp"

#include "graphics/Pipeline.hpp"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "core/Formatters.hpp"
#include "core/Types.hpp"
#include "core/filesystem/FileIO.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/PushConstantLayout.hpp"
#include "graphics/RenderPass.hpp"
#include "util/Timer.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Image.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/Shader.hpp"

namespace Disarray::Vulkan {

namespace Detail {

	static constexpr auto to_vulkan_format(ElementType type) -> VkFormat
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
		case ElementType::Uint:
			return VK_FORMAT_R32_UINT;
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

	static constexpr auto vk_polygon_mode(PolygonMode mode) -> VkPolygonMode
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

	static constexpr auto vk_polygon_topology(PolygonMode mode) -> VkPrimitiveTopology
	{
		switch (mode) {
		case PolygonMode::Fill:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case PolygonMode::Line:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case PolygonMode::Point:
			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		default:
			unreachable();
		}
	}

	static constexpr auto to_vulkan_comparison(DepthCompareOperator depth_comp_operator) -> VkCompareOp
	{
		switch (depth_comp_operator) {
		case DepthCompareOperator::None:
		case DepthCompareOperator::Never:
			return VK_COMPARE_OP_NEVER;
		case DepthCompareOperator::NotEqual:
			return VK_COMPARE_OP_NOT_EQUAL;
		case DepthCompareOperator::Less:
			return VK_COMPARE_OP_LESS;
		case DepthCompareOperator::LessOrEqual:
			return VK_COMPARE_OP_LESS_OR_EQUAL;
		case DepthCompareOperator::Greater:
			return VK_COMPARE_OP_GREATER;
		case DepthCompareOperator::GreaterOrEqual:
			return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case DepthCompareOperator::Equal:
			return VK_COMPARE_OP_EQUAL;
		case DepthCompareOperator::Always:
			return VK_COMPARE_OP_ALWAYS;
		default:
			unreachable();
		}
	}

	static constexpr auto to_vulkan_cull_mode(CullMode cull) -> VkCullModeFlags
	{
		switch (cull) {
		case CullMode::Front:
			return VK_CULL_MODE_FRONT_BIT;
		case CullMode::Back:
			return VK_CULL_MODE_BACK_BIT;
		case CullMode::Both:
			return VK_CULL_MODE_FRONT_AND_BACK;
		case CullMode::None:
			return VK_CULL_MODE_NONE;
		default:
			unreachable();
		}
	}

	static constexpr auto to_vulkan_face_mode(FaceMode face_mode) -> VkFrontFace
	{
		switch (face_mode) {
		case FaceMode::Clockwise:
			return VK_FRONT_FACE_CLOCKWISE;
		case FaceMode::CounterClockwise:
			return VK_FRONT_FACE_COUNTER_CLOCKWISE;
		default:
			unreachable();
		}
	}

} // namespace Detail

Pipeline::Pipeline(const Disarray::Device& dev, Disarray::PipelineProperties properties)
	: Disarray::Pipeline(std::move(properties))
	, device(dev)
{
	if (props.framebuffer) {
		props.framebuffer->register_on_framebuffer_change(
			[this](Disarray::Framebuffer& frame_buffer) { recreate_pipeline(true, frame_buffer.get_properties().extent); });
	}

	recreate_pipeline(false, {});
}

auto Pipeline::initialise_blend_states() -> std::vector<VkPipelineColorBlendAttachmentState>
{
	const auto& fb_props = props.framebuffer->get_properties();
	const auto should_present = fb_props.should_present;
	std::size_t color_attachment_count = should_present ? 1 : props.framebuffer->get_colour_attachment_count();
	std::vector<VkPipelineColorBlendAttachmentState> blend_attachment_states(color_attachment_count);
	static constexpr auto blend_all_factors
		= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	if (should_present) {
		blend_attachment_states[0].colorWriteMask = blend_all_factors;
		blend_attachment_states[0].blendEnable = static_cast<VkBool32>(true);
		blend_attachment_states[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blend_attachment_states[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blend_attachment_states[0].colorBlendOp = VK_BLEND_OP_ADD;
		blend_attachment_states[0].alphaBlendOp = VK_BLEND_OP_ADD;
		blend_attachment_states[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blend_attachment_states[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

		return blend_attachment_states;
	}
	for (size_t i = 0; i < color_attachment_count; i++) {
		blend_attachment_states[i].colorWriteMask = blend_all_factors;

		if (!fb_props.should_blend) {
			blend_attachment_states[i].blendEnable = VK_FALSE;
			break;
		}

		const auto& attachment_spec = fb_props.attachments[i];
		FramebufferBlendMode blend_mode = fb_props.blend_mode == FramebufferBlendMode::None ? attachment_spec.blend_mode : fb_props.blend_mode;

		blend_attachment_states[i].blendEnable = static_cast<VkBool32>(attachment_spec.blend);
		blend_attachment_states[i].colorBlendOp = VK_BLEND_OP_ADD;
		blend_attachment_states[i].alphaBlendOp = VK_BLEND_OP_ADD;
		blend_attachment_states[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blend_attachment_states[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

		switch (blend_mode) {
		case FramebufferBlendMode::SrcAlphaOneMinusSrcAlpha:
			blend_attachment_states[i].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			blend_attachment_states[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			blend_attachment_states[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			blend_attachment_states[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			break;
		case FramebufferBlendMode::OneZero:
			blend_attachment_states[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			blend_attachment_states[i].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			break;
		case FramebufferBlendMode::Zero_SrcColor:
			blend_attachment_states[i].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			blend_attachment_states[i].dstColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
			break;

		default:
			unreachable();
		}
	}

	return blend_attachment_states;
}

void Pipeline::construct_layout(const Extent& extent)
{
	if (cache == nullptr) {
		try_find_or_recreate_cache();
	}

	std::vector<VkDynamicState> dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_LINE_WIDTH };

	VkPipelineDynamicStateCreateInfo dynamic_state {};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = static_cast<std::uint32_t>(dynamic_states.size());
	dynamic_state.pDynamicStates = dynamic_states.data();

	VkPipelineVertexInputStateCreateInfo vertex_input_info {};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	const auto& bindings = props.layout.construct_binding();
	VkVertexInputBindingDescription binding_description {};
	binding_description.binding = bindings.binding;
	binding_description.stride = bindings.stride;
	binding_description.inputRate = bindings.input_rate == InputRate::Vertex ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;

	std::vector<VkVertexInputAttributeDescription> attribute_descriptions {};
	std::uint32_t location = 0;
	for (const auto& attribute : props.layout.elements) {
		auto& new_attribute = attribute_descriptions.emplace_back();
		new_attribute.binding = 0;
		new_attribute.format = Detail::to_vulkan_format(attribute.type);
		new_attribute.offset = attribute.offset;
		new_attribute.location = location++;
	}

	vertex_input_info.vertexBindingDescriptionCount = 1;
	vertex_input_info.pVertexBindingDescriptions = &binding_description;
	vertex_input_info.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attribute_descriptions.size());
	vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

	VkPipelineInputAssemblyStateCreateInfo input_assembly {};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = Detail::vk_polygon_topology(props.polygon_mode);
	input_assembly.primitiveRestartEnable = static_cast<VkBool32>(false);

	// Prefer extent over props (due to resizing API)
	std::uint32_t width { props.extent.width };
	std::uint32_t height { props.extent.height };
	if (extent.valid()) {
		width = extent.width;
		height = extent.height;
	}

	VkViewport viewport {};
	viewport.x = 0.0F;
	viewport.y = 0.0F;
	viewport.width = static_cast<float>(width);
	viewport.height = static_cast<float>(height);
	viewport.minDepth = 0.0F;
	viewport.maxDepth = 1.0F;

	VkRect2D scissor {};
	scissor.offset = { 0, 0 };
	scissor.extent = {
		.width = static_cast<std::uint32_t>(width),
		.height = static_cast<std::uint32_t>(height),
	};

	VkPipelineViewportStateCreateInfo viewport_state {};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &viewport;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = static_cast<VkBool32>(false);
	rasterizer.rasterizerDiscardEnable = static_cast<VkBool32>(false);
	rasterizer.polygonMode = Detail::vk_polygon_mode(props.polygon_mode);
	rasterizer.lineWidth = props.line_width;
	rasterizer.cullMode = Detail::to_vulkan_cull_mode(props.cull_mode);
	rasterizer.frontFace = Detail::to_vulkan_face_mode(props.face_mode);
	rasterizer.depthBiasEnable = static_cast<VkBool32>(false);
	rasterizer.depthBiasConstantFactor = 0.0F; // Optional
	rasterizer.depthBiasClamp = 0.0F; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0F; // Optional

	auto depth_stencil_state_create_info = vk_structures<VkPipelineDepthStencilStateCreateInfo> {}();
	depth_stencil_state_create_info.depthTestEnable = static_cast<VkBool32>(props.test_depth);
	depth_stencil_state_create_info.depthWriteEnable = static_cast<VkBool32>(props.write_depth);
	depth_stencil_state_create_info.depthCompareOp = Detail::to_vulkan_comparison(props.depth_comparison_operator);
	depth_stencil_state_create_info.depthBoundsTestEnable = static_cast<VkBool32>(false);
	depth_stencil_state_create_info.stencilTestEnable = static_cast<VkBool32>(false);

	VkPipelineMultisampleStateCreateInfo multisampling {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = static_cast<VkBool32>(false);
	multisampling.rasterizationSamples = to_vulkan_samples(props.samples);
	multisampling.minSampleShading = 1.0F; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = static_cast<VkBool32>(false); // Optional
	multisampling.alphaToOneEnable = static_cast<VkBool32>(false); // Optional

	VkPipelineColorBlendStateCreateInfo color_blending {};
	auto blend_states = initialise_blend_states();
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.logicOpEnable = static_cast<VkBool32>(props.framebuffer->get_properties().should_blend);
	color_blending.pAttachments = blend_states.data();
	color_blending.attachmentCount = static_cast<std::uint32_t>(blend_states.size());

	VkPipelineLayoutCreateInfo pipeline_layout_info {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = static_cast<std::uint32_t>(props.descriptor_set_layouts.size()); // Optional
	pipeline_layout_info.pSetLayouts = props.descriptor_set_layouts.data(); // Optional

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
	pipeline_layout_info.pushConstantRangeCount = static_cast<std::uint32_t>(result.size()); // Optional
	pipeline_layout_info.pPushConstantRanges = result.data(); // Optional

	verify(vkCreatePipelineLayout(supply_cast<Vulkan::Device>(device), &pipeline_layout_info, nullptr, &layout));

	VkGraphicsPipelineCreateInfo pipeline_create_info {};
	pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	auto stages = retrieve_shader_stages(props.vertex_shader, props.fragment_shader);
	std::vector<VkPipelineShaderStageCreateInfo> stage_data { stages.first, stages.second };
	pipeline_create_info.pStages = stage_data.data();
	pipeline_create_info.stageCount = static_cast<std::uint32_t>(stage_data.size());
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

	verify(vkCreateGraphicsPipelines(supply_cast<Vulkan::Device>(device), cache, 1, &pipeline_create_info, nullptr, &pipeline));
}

Pipeline::~Pipeline()
{
	vkDestroyPipelineLayout(supply_cast<Vulkan::Device>(device), layout, nullptr);
	vkDestroyPipeline(supply_cast<Vulkan::Device>(device), pipeline, nullptr);

	if (cache == nullptr) {
		return;
	}

	std::size_t size { 0 };
	vkGetPipelineCacheData(supply_cast<Vulkan::Device>(device), cache, &size, nullptr);

	std::vector<const void*> data;
	data.resize(size);
	vkGetPipelineCacheData(supply_cast<Vulkan::Device>(device), cache, &size, data.data());

	const auto pipeline_name = fmt::format(
		"Pipeline-{}-{}", props.vertex_shader->get_properties().identifier.filename(), props.fragment_shader->get_properties().identifier.filename());

	const auto name = fmt::format("Assets/Pipelines/{}-Cache-{}.pipe-bin", pipeline_name, props.hash());
	FS::write_to_file(name, size, std::span { data });

	vkDestroyPipelineCache(supply_cast<Vulkan::Device>(device), cache, nullptr);
}

auto Pipeline::retrieve_shader_stages(Ref<Disarray::Shader> vertex, Ref<Disarray::Shader> fragment)
	-> std::pair<VkPipelineShaderStageCreateInfo, VkPipelineShaderStageCreateInfo>
{
	return { supply_cast<Vulkan::Shader>(*vertex), supply_cast<Vulkan::Shader>(*fragment) };
}

void Pipeline::recreate(bool should_clean, const Extent& extent) { recreate_pipeline(should_clean, extent); }

void Pipeline::recreate_pipeline(bool should_clean, const Extent& extent)
{
	if (should_clean) {
		vkDestroyPipelineLayout(supply_cast<Vulkan::Device>(device), layout, nullptr);
		vkDestroyPipeline(supply_cast<Vulkan::Device>(device), pipeline, nullptr);
	}

	if (is_valid()) {
		construct_layout(extent);
	}
}

auto Pipeline::get_framebuffer() -> Disarray::Framebuffer& { return *props.framebuffer; }

auto Pipeline::get_render_pass() -> Disarray::RenderPass& { return props.framebuffer->get_render_pass(); }

void Pipeline::try_find_or_recreate_cache()
{
	const auto pipeline_name = fmt::format(
		"Pipeline-{}-{}", props.vertex_shader->get_properties().identifier.filename(), props.fragment_shader->get_properties().identifier.filename());

	const auto name = fmt::format("Assets/Pipelines/{}-Cache-{}.pipe-bin", pipeline_name, props.hash());

	std::ifstream input_stream { name, std::fstream::ate | std::fstream::binary };
	if (!input_stream) {
		VkPipelineCacheCreateInfo cache_create_info {};
		cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		vkCreatePipelineCache(supply_cast<Vulkan::Device>(device), &cache_create_info, nullptr, &cache);
		return;
	}

	MSTimer timer;
	std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input_stream), {});

	VkPipelineCacheCreateInfo cache_create_info {};
	cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	cache_create_info.pInitialData = buffer.data();
	cache_create_info.initialDataSize = buffer.size() * sizeof(unsigned char);
	vkCreatePipelineCache(supply_cast<Vulkan::Device>(device), &cache_create_info, nullptr, &cache);

	Log::info("Pipeline", "Loading cache with id {} took {}s", name, timer.elapsed());
}

} // namespace Disarray::Vulkan
