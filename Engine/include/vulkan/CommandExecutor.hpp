#pragma once

#include "graphics/PhysicalDevice.hpp"
#include "graphics/QueueFamilyIndex.hpp"
#include "graphics/Surface.hpp"
#include "vulkan/PropertySupplier.hpp"
#include "vulkan/QueueFamilyIndex.hpp"
#include "vulkan/Swapchain.hpp"
#include "graphics/CommandExecutor.hpp"

#include <vector>

namespace Disarray::Vulkan {

	class CommandExecutor: public Disarray::CommandExecutor, public PropertySupplier<VkCommandBuffer>
	{
	public:
		CommandExecutor(Ref<Disarray::Device>, Ref<Disarray::Swapchain>, Ref<Disarray::QueueFamilyIndex>, const Disarray::CommandExecutorProperties&);
		~CommandExecutor() override;

		void begin() override;
		void end() override;
		void submit_and_end() override;

		void force_recreation() override { recreate(); }

		VkCommandBuffer supply() const override { return active; }

	private:
		void recreate(bool should_clean = true);

		Ref<Device> device;
		Ref<Disarray::Swapchain> swapchain;
		Ref<Disarray::QueueFamilyIndex> indexes;

		CommandExecutorProperties props;
		std::uint32_t current {0};
		VkCommandPool command_pool;
		std::vector<VkCommandBuffer> command_buffers;
		VkCommandBuffer active {nullptr};
		std::vector<VkFence> fences;
		VkQueue graphics_queue;
	};

}