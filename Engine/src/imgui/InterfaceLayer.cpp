#include "DisarrayPCH.hpp"

#include "core/Types.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Device.hpp"
#include "graphics/Renderer.hpp"
#include "ui/InterfaceLayer.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/DebugMarker.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Instance.hpp"
#include "vulkan/PhysicalDevice.hpp"

#include <imgui.h>
#include <vulkan/vulkan.h>

namespace Disarray::UI {

struct InterfaceLayer::RendererSpecific {
	Device& device;

	explicit RendererSpecific(Device& dev)
		: device(dev)
	{
	}

	VkDescriptorPool pool { nullptr };

	~RendererSpecific() { vkDestroyDescriptorPool(supply_cast<Vulkan::Device>(device), pool, nullptr); }
};

InterfaceLayer::InterfaceLayer(Device& dev, Window& win, Swapchain& swap)
	: device(dev)
	, window(win)
	, swapchain(swap)
{
	pimpl = std::make_unique<RendererSpecific>(device);
}

InterfaceLayer::~InterfaceLayer() { }

void InterfaceLayer::construct(App&, Renderer& renderer, ThreadPool&)
{
	command_executor = CommandExecutor::construct(device, swapchain,
		{
			.count = 3,
			.is_primary = false,
			.owned_by_swapchain = false,
		});
	std::array<VkDescriptorPoolSize, 11> pool_sizes = { VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 }, { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 }, { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 }, { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 }, { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 }, { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 }, { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

	auto pool_info = Vulkan::vk_structures<VkDescriptorPoolCreateInfo> {}();
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = static_cast<std::uint32_t>(pool_sizes.size()) * 11ul;
	pool_info.poolSizeCount = static_cast<std::uint32_t>(std::size(pool_sizes));
	pool_info.pPoolSizes = pool_sizes.data();

	auto& vk_device = cast_to<Vulkan::Device>(device);
	Vulkan::verify(vkCreateDescriptorPool(*vk_device, &pool_info, nullptr, &pimpl->pool));
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
	// io.ConfigViewportsNoDecoration = false;
	// io.ConfigViewportsNoAutoMerge = true;
	// io.ConfigViewportsNoTaskBarIcon = true;

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, style.Colors[ImGuiCol_WindowBg].w);

	ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow*>(window.native()), true);

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = supply_cast<Vulkan::Instance>(window.get_instance());
	init_info.PhysicalDevice = supply_cast<Vulkan::PhysicalDevice>(device.get_physical_device());
	init_info.Device = *vk_device;
	init_info.Queue = vk_device.get_graphics_queue();
	init_info.DescriptorPool = pimpl->pool;
	init_info.MinImageCount = 2;
	init_info.ImageCount = 3;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&init_info, supply_cast<Vulkan::RenderPass>(swapchain.get_render_pass()));

	{
		auto executor = Vulkan::construct_immediate(device);
		ImGui_ImplVulkan_CreateFontsTexture(executor->supply());
	}

	// clear font textures from cpu data
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void InterfaceLayer::handle_swapchain_recreation(Swapchain&) { command_executor->recreate(true, {}); }

void InterfaceLayer::on_event(Event& event)
{
	for (auto& panel : panels) {
		panel->on_event(event);
	}
}

void InterfaceLayer::update(float ts)
{
	for (auto& panel : panels) {
		panel->update(ts);
	}
}

void InterfaceLayer::destruct()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
};

void InterfaceLayer::interface()
{
	for (auto& panel : panels) {
		panel->interface();
	}
}

void InterfaceLayer::begin()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void InterfaceLayer::end()
{
	ImGui::Render();
	static constexpr VkClearColorValue clear_colour { { 0.1f, 0.1f, 0.1f, 0.0f } };
	static constexpr VkClearDepthStencilValue depth_stencil_clear { .depth = 1.0f, .stencil = 0 };

	std::array<VkClearValue, 2> clear_values {};
	clear_values[0].color = clear_colour;
	clear_values[1].depthStencil = depth_stencil_clear;

	const auto& [width, height] = swapchain.get_extent();

	auto& vk_swapchain = cast_to<Vulkan::Swapchain>(swapchain);
	const VkCommandBuffer draw_command_buffer = vk_swapchain.get_drawbuffer();

	auto begin_info = Vulkan::vk_structures<VkCommandBufferBeginInfo> {}();
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	begin_info.pNext = nullptr;
	vkBeginCommandBuffer(draw_command_buffer, &begin_info);

	auto vk_render_pass = supply_cast<Vulkan::RenderPass>(vk_swapchain.get_render_pass());
	auto vk_framebuffer = vk_swapchain.get_current_framebuffer();

	VkRenderPassBeginInfo render_pass_begin_info = {};
	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.renderPass = vk_render_pass;
	render_pass_begin_info.renderArea.offset.x = 0;
	render_pass_begin_info.renderArea.offset.y = 0;
	render_pass_begin_info.renderArea.extent.width = width;
	render_pass_begin_info.renderArea.extent.height = height;
	render_pass_begin_info.clearValueCount = static_cast<std::uint32_t>(clear_values.size());
	render_pass_begin_info.pClearValues = clear_values.data();
	render_pass_begin_info.framebuffer = vk_framebuffer;

	Vulkan::DebugMarker::begin_region(draw_command_buffer, "Interface", { 1, 0, 0, 1 });

	vkCmdBeginRenderPass(draw_command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	auto& imgui_buffer = cast_to<Vulkan::CommandExecutor>(*command_executor);
	{
		VkCommandBufferInheritanceInfo inheritance_info = {};
		inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritance_info.renderPass = vk_render_pass;
		inheritance_info.framebuffer = vk_framebuffer;

		VkCommandBufferBeginInfo cbi = {};
		cbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cbi.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		cbi.pInheritanceInfo = &inheritance_info;
		imgui_buffer.begin(cbi);

		const auto& command_buffer = imgui_buffer.supply();
		VkViewport viewport {};
		viewport.x = 0.0f;
		viewport.y = static_cast<float>(height);
		viewport.height = static_cast<float>(height);
		viewport.width = static_cast<float>(width);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(command_buffer, 0, 1, &viewport);

		VkRect2D scissor {};
		scissor.extent.width = width;
		scissor.extent.height = height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(command_buffer, 0, 1, &scissor);

		static float scale_x { -1.0f };
		static float scale_y { -1.0f };
		static bool scaled_already { false };

		ImDrawData* main_draw_data = ImGui::GetDrawData();
		if (!scaled_already) {
			const auto&& [sx, sy] = std::make_pair(1.f, 1.f);
			scale_x = sx;
			scale_y = sy;
			scaled_already = true;
		}

		main_draw_data->FramebufferScale = { scale_x, scale_y };
		// UI scale and translate via push constants
		ImGui_ImplVulkan_RenderDrawData(main_draw_data, command_buffer);

		imgui_buffer.end();
	}

	std::array<VkCommandBuffer, 1> buffer { imgui_buffer.supply() };
	vkCmdExecuteCommands(draw_command_buffer, 1, buffer.data());

	Vulkan::DebugMarker::end_region(draw_command_buffer);
	vkCmdEndRenderPass(draw_command_buffer);

	Vulkan::verify(vkEndCommandBuffer(draw_command_buffer));

	if (const ImGuiIO& io = ImGui::GetIO(); io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void InterfaceLayer::render(Renderer& renderer)
{
	for (auto& panel : panels) {
		panel->render(renderer);
	}
}
} // namespace Disarray::UI
