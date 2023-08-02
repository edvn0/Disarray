#pragma once

#include "core/ReferenceCounted.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Device.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "graphics/QueueFamilyIndex.hpp"
#include "graphics/Surface.hpp"
#include "vulkan/PropertySupplier.hpp"
#include "vulkan/QueueFamilyIndex.hpp"
#include "vulkan/Swapchain.hpp"

#include <vector>

namespace Disarray::Vulkan {

	class CommandExecutor : public Disarray::CommandExecutor, public PropertySupplier<VkCommandBuffer> {
	public:
		CommandExecutor(Disarray::Device&, Disarray::Swapchain&, const Disarray::CommandExecutorProperties&);
		~CommandExecutor() override;

		void begin() override;
		void end() override;
		void submit_and_end() override;

		void begin(VkCommandBufferBeginInfo);

		void force_recreation() override { recreate(); }

		VkCommandBuffer supply() const override { return active; }

		auto buffer_index() -> std::uint32_t
		{
			// Frame dependent buffer
			if (is_frame_dependent_executor) {
				return current;
			}

			// This buffer is owned by the swapchain, technically never called
			if (props.owned_by_swapchain) {
				return swapchain.get_current_frame();
			}

			// Immediate buffer.
			return 0;
		}

	private:
		void recreate(bool should_clean = true);

		Disarray::Device& device;
		Disarray::Swapchain& swapchain;
		Disarray::QueueFamilyIndex& indexes;
		CommandExecutorProperties props;
		bool is_frame_dependent_executor { false };

		std::uint32_t current { 0 };
		std::uint32_t image_count { 0 };
		VkCommandPool command_pool;
		std::vector<VkCommandBuffer> command_buffers;
		VkCommandBuffer active { nullptr };
		std::vector<VkFence> fences;
		VkQueue graphics_queue;
	};

} // namespace Disarray::Vulkan
