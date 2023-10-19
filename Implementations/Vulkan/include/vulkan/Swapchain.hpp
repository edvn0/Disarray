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

	auto image_count() const -> std::uint32_t override { return static_cast<std::uint32_t>(swapchain_images.size()); }
	auto get_extent() const -> Disarray::Extent override { return { extent.width, extent.height }; }
	auto get_samples() const -> SampleCount override { return samples; }

	auto get_current_frame() const -> std::uint32_t override { return current_frame; }
	auto advance_frame() -> std::uint32_t override { return current_frame++; }
	auto get_image_index() const -> std::uint32_t override { return image_index; }
	auto get_framebuffer() { return framebuffers[get_current_frame()]; }

	auto get_drawbuffer() const -> VkCommandBuffer { return command_buffers[get_current_frame()].buffer; }
	auto get_drawbuffer() -> VkCommandBuffer { return command_buffers[get_current_frame()].buffer; }

	auto get_render_pass() -> Disarray::RenderPass& override;
	auto get_current_framebuffer() -> VkFramebuffer { return framebuffers[get_current_frame()]; };

	auto prepare_frame() -> bool override;
	void present() override;

	auto needs_recreation() const -> bool override { return swapchain_needs_recreation; }
	void reset_recreation_status() override { swapchain_needs_recreation = false; }

	auto get_views() const -> const auto& { return swapchain_image_views; }

	auto supply() const -> VkSwapchainKHR override { return swapchain; }

	auto get_image_format() const { return format.format; }

private:
	void create_synchronisation_objects();
	void recreate_swapchain(Disarray::Swapchain* old = nullptr, bool should_clean = true);
	void recreate_framebuffer();
	void cleanup_swapchain();
	void recreate_renderpass();

	bool swapchain_needs_recreation { false };

	Disarray::Window& window;
	Disarray::Device& device;
	VkSwapchainKHR swapchain {};

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

	VkQueue present_queue {};
	VkQueue graphics_queue {};
	VkSurfaceFormatKHR format {};
	VkPresentModeKHR present_mode {};
	VkExtent2D extent {};
	uint32_t graphics_family {};
};
} // namespace Disarray::Vulkan
