#pragma once

#include <vector>

#include "Forward.hpp"
#include "core/Window.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/Swapchain.hpp"
#include "vulkan/Framebuffer.hpp"
#include "vulkan/PropertySupplier.hpp"
#include "vulkan/RenderPass.hpp"

namespace Disarray::Vulkan {

class CommandExecutor;

class Swapchain : public Disarray::Swapchain, public PropertySupplier<VkSwapchainKHR> {
public:
	Swapchain(Disarray::Window&, Disarray::Device&, Disarray::Swapchain* = nullptr);
	~Swapchain() override;

	std::uint32_t image_count() const override { return static_cast<std::uint32_t>(swapchain_images.size()); }
	Disarray::Extent get_extent() const override { return { extent.width, extent.height }; }
	SampleCount get_samples() const override { return samples; }

	std::uint32_t get_current_frame() const override { return current_frame; }
	std::uint32_t advance_frame() override { return current_frame++; }
	std::uint32_t get_image_index() const override { return image_index; }
	auto get_framebuffer() { return framebuffers[get_current_frame()]; }

	VkCommandBuffer get_drawbuffer() const { return command_buffers[get_current_frame()].buffer; }
	VkCommandBuffer get_drawbuffer() { return command_buffers[get_current_frame()].buffer; }

	Disarray::RenderPass& get_render_pass() override;
	VkFramebuffer get_current_framebuffer() { return framebuffers[get_current_frame()]; };

	bool prepare_frame() override;
	void present() override;

	bool needs_recreation() const override { return swapchain_needs_recreation; }
	void reset_recreation_status() override { swapchain_needs_recreation = false; }

	const auto& get_views() const { return swapchain_image_views; }

	VkSwapchainKHR supply() const override { return swapchain; }

private:
	void create_synchronisation_objects();
	void recreate_swapchain(Disarray::Swapchain* old = nullptr, bool should_clean = true);
	void recreate_framebuffer();
	void cleanup_swapchain();
	void recreate_renderpass();

	bool swapchain_needs_recreation { false };

	Disarray::Window& window;
	Disarray::Device& device;
	VkSwapchainKHR swapchain;

	std::uint32_t current_frame { 0 };
	std::uint32_t image_index { 0 };

	std::vector<VkFramebuffer> framebuffers;
	Ref<Disarray::RenderPass> render_pass { nullptr };

	SampleCount samples { SampleCount::One };

	std::vector<VkImage> swapchain_images;
	std::vector<VkImageView> swapchain_image_views;
	std::vector<VkSemaphore> image_available_semaphores;
	std::vector<VkSemaphore> render_finished_semaphores;
	std::vector<VkFence> in_flight_fences;

	struct CommandBuffer {
		VkCommandBuffer buffer;
		VkCommandPool command_pool;
	};
	std::vector<CommandBuffer> command_buffers;

	VkQueue present_queue;
	VkQueue graphics_queue;
	VkSurfaceFormatKHR format;
	VkPresentModeKHR present_mode;
	VkExtent2D extent;
	uint32_t graphics_family;
};
} // namespace Disarray::Vulkan
