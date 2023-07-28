#pragma once

#include "core/Window.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "graphics/Swapchain.hpp"
#include "vulkan/PropertySupplier.hpp"
#include "vulkan/vulkan_core.h"

#include <vector>

namespace Disarray::Vulkan {

	struct PresentingSemaphores {
		VkSemaphore& image_available;
		VkSemaphore& render_finished;
	};

	class Swapchain : public Disarray::Swapchain, public PropertySupplier<VkSwapchainKHR> {
	public:
		Swapchain(Scope<Disarray::Window>&, Ref<Disarray::Device>, Ref<Disarray::PhysicalDevice>, Ref<Disarray::Swapchain> = nullptr);
		~Swapchain() override;

		std::uint32_t image_count() const override { return swapchain_images.size(); }
		Disarray::Extent get_extent() const override { return { extent.width, extent.height }; }

		std::uint32_t get_current_frame() override { return current_frame; }
		std::uint32_t advance_frame() override { return current_frame++; }
		std::uint32_t get_image_index() override { return image_index; }

		VkCommandBuffer get_drawbuffer() { return command_buffers[get_image_index()]; }

		PresentingSemaphores get_presenting_semaphores() {
			return { image_available_semaphores[current_frame], render_finished_semaphores[current_frame] };
		}

		bool prepare_frame() override;
		void present() override;

		bool needs_recreation() override { return swapchain_needs_recreation; }
		void reset_recreation_status() override {swapchain_needs_recreation =false; }

		const auto& get_views() const { return swapchain_image_views; }

		VkSwapchainKHR supply() const override { return swapchain; }

	private:
		void create_synchronisation_objects();
		void recreate_swapchain(Ref<Disarray::Swapchain> old = nullptr, bool should_clean = true);
		void cleanup_swapchain();

		bool swapchain_needs_recreation {false};

		Ref<Disarray::Device> device;
		Scope<Disarray::Window>& window;
		Ref<Disarray::PhysicalDevice> physical_device;
		VkSwapchainKHR swapchain;

		std::uint32_t current_frame {0};
		std::uint32_t image_index {0};

		std::vector<VkImage> swapchain_images;
		std::vector<VkImageView> swapchain_image_views;
		std::vector<VkSemaphore> image_available_semaphores;
		std::vector<VkSemaphore> render_finished_semaphores;
		std::vector<VkFence> in_flight_fences;

		VkCommandPool command_pool;
		std::vector<VkCommandBuffer> command_buffers;

		VkQueue present_queue;
		VkQueue graphics_queue;
		VkSurfaceFormatKHR format;
		VkPresentModeKHR present_mode;
		VkExtent2D extent;
		uint32_t graphics_family;
	};
} // namespace Disarray::Vulkan