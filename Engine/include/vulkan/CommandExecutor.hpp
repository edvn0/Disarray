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

	struct PipelineStatistics {
		uint64_t input_assembly_vertices { 0 };
		uint64_t input_assembly_primitives { 0 };
		uint64_t vertex_shader_invocations { 0 };
		uint64_t clipping_invocations { 0 };
		uint64_t clipping_primitives { 0 };
		uint64_t fragment_shader_invocations { 0 };
		uint64_t compute_shader_invocations { 0 };
	};

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
	};

	template <typename T>
		requires(std::is_base_of_v<CommandExecutor, T>)
	static std::tuple<Ref<T>, std::function<void(Ref<T>&)>> construct_immediate(Disarray::Device& device, Disarray::Swapchain& swapchain)
	{
		static constexpr Disarray::CommandExecutorProperties props { .count = 1, .owned_by_swapchain = false };
		auto executor = CommandExecutor::construct_as<T>(device, swapchain, props);
		executor->begin();
		auto destructor = [&device = device](Ref<T>& exec) {
			exec->submit_and_end();
			wait_for_cleanup(device);
			exec.reset();
		};
		return { executor, destructor };
	}

} // namespace Disarray::Vulkan
