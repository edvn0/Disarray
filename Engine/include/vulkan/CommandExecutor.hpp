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

#include <tuple>
#include <type_traits>
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

		void force_recreation() override { recreate_executor(); }
		void recreate(bool should_clean) override { return recreate_executor(should_clean); }

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

		void wait_indefinite();

		float get_gpu_execution_time(uint32_t frame_index, uint32_t query_index = 0) const override
		{
			if (query_index == UINT32_MAX || query_index / 2 >= timestamp_next_available_query / 2)
				return 0.0f;

			return execution_gpu_times[frame_index][query_index / 2];
		}

		const PipelineStatistics& get_pipeline_statistics(uint32_t frame_index) const override
		{
			return pipeline_statistics_query_results[frame_index];
		}

	private:
		void recreate_executor(bool should_clean = true);
		void create_query_pools();
		void record_stats();
		void destroy_executor();

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

		std::uint32_t timestamp_query_count { 0 };
		uint32_t timestamp_next_available_query { 2 };
		std::vector<VkQueryPool> timestamp_query_pools;
		std::vector<VkQueryPool> pipeline_statistics_query_pools;
		std::vector<std::vector<uint64_t>> timestamp_query_results;
		std::vector<std::vector<float>> execution_gpu_times;

		std::uint32_t pipeline_query_count { 0 };
		std::vector<PipelineStatistics> pipeline_statistics_query_results;
		void create_base_structures();
	};

	namespace {
		void submit_and_delete_executor(Vulkan::CommandExecutor* to_destroy)
		{
			to_destroy->submit_and_end();
			to_destroy->wait_indefinite();
			delete to_destroy;
		}
	} // namespace

	constexpr auto deleter = [](Vulkan::CommandExecutor* l) { submit_and_delete_executor(l); };
	using ImmediateExecutor = std::unique_ptr<Vulkan::CommandExecutor, decltype(deleter)>;
	ImmediateExecutor construct_immediate(Disarray::Device& device, Disarray::Swapchain& swapchain)
	{
		static constexpr Disarray::CommandExecutorProperties props { .count = 1, .owned_by_swapchain = false };
		ImmediateExecutor executor { new CommandExecutor { device, swapchain, props } };
		executor->begin();
		return executor;
	}

} // namespace Disarray::Vulkan
