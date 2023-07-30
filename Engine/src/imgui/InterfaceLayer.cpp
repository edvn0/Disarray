#include "ui/InterfaceLayer.hpp"

#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"
#include "core/Types.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Image.hpp"
#include "graphics/ImageProperties.hpp"
#include "graphics/RenderPass.hpp"
#include "imgui.h"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/PhysicalDevice.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/Surface.hpp"
#include "vulkan/Verify.hpp"
#include "vulkan/Window.hpp"

#include "vulkan/Renderer.hpp"
#include "vulkan/vulkan_core.h"

#include <vulkan/vulkan.h>

namespace Disarray::UI {

	static VkDescriptorPool pool;

	InterfaceLayer::InterfaceLayer(Ref<Device> dev, Ref<PhysicalDevice> phy, Scope<Window>& win, Ref<Swapchain> swap)
		: device(dev)
		, physical_device(phy)
		, window(win)
		, swapchain(swap)
	{
	}

	InterfaceLayer::~InterfaceLayer()
	{
		vkDestroyDescriptorPool(supply_cast<Vulkan::Device>(device), pool, nullptr);
	}

	void InterfaceLayer::construct(App&, Renderer&)
	{
		// 1: create descriptor pool for IMGUI
		//  the size of the pool is very oversize, but it's copied from imgui demo itself.
		VkDescriptorPoolSize pool_sizes[] =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
			};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = std::size(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;

		auto vk_device = cast_to<Vulkan::Device>(device);
		Vulkan::verify(vkCreateDescriptorPool(vk_device->supply(), &pool_info, nullptr, &pool));
		ImGui::CreateContext();

		// 2: initialize imgui library
		ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow*>(window->native()), true);

		// this initializes the core structures of imgui

		// this initializes imgui for Vulkan
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = supply_cast<Vulkan::Instance>(window->get_instance());
		init_info.PhysicalDevice = supply_cast<Vulkan::PhysicalDevice>(physical_device);
		init_info.Device = vk_device->supply();
		init_info.Queue = vk_device->get_graphics_queue();
		init_info.DescriptorPool = pool;
		init_info.MinImageCount = 3;
		init_info.ImageCount = 3;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		render_pass = make_ref<Vulkan::RenderPass>(device, RenderPassProperties {ImageFormat::SBGR});

		ImGui_ImplVulkan_Init(&init_info, supply_cast<Vulkan::RenderPass>(render_pass));

		// execute a gpu command to upload imgui font textures
		auto&& [immediate, destructor] = construct_immediate<Vulkan::CommandExecutor>(device, swapchain, physical_device->get_queue_family_indexes());
		ImGui_ImplVulkan_CreateFontsTexture(immediate->supply());
		destructor(immediate);

		//clear font textures from cpu data
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	void InterfaceLayer::handle_swapchain_recreation(Renderer&) {

	}

	void InterfaceLayer::update(float ts) {

	}

	void InterfaceLayer::update(float ts, Renderer& renderer) {
		static bool is_open {true};
		ImGui::ShowDemoWindow(&is_open);
	}

	void InterfaceLayer::destruct() {
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	};

	void InterfaceLayer::begin()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void InterfaceLayer::end(Renderer& renderer)
	{
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), supply_cast<Vulkan::CommandExecutor>(renderer.get_current_executor()));
	}
}