#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>

#include "core/DisarrayObject.hpp"
#include "core/PointerDefinition.hpp"
#include "core/Types.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Device.hpp"
#include "graphics/QueueFamilyIndex.hpp"
#include "vulkan/PropertySupplier.hpp"
#include "vulkan/Swapchain.hpp"

namespace Disarray::Vulkan {

class CommandExecutor : public Disarray::CommandExecutor, public PropertySupplier<VkCommandBuffer> {
	DISARRAY_MAKE_NONCOPYABLE(CommandExecutor)
public:
	CommandExecutor(const Disarray::Device&, const Disarray::Swapchain*, Disarray::CommandExecutorProperties);
	~CommandExecutor() override;

	void begin() override;
	void end() override;
	void submit_and_end() override;

	void begin(VkCommandBufferBeginInfo);

	void force_recreation() override { recreate_executor(); }
	void recreate(bool should_clean, const Extent&) override { return recreate_executor(should_clean); }

	auto supply() const -> VkCommandBuffer override { return active; }
	auto supply() -> VkCommandBuffer override { return active; }

	auto get_buffer(std::uint32_t index) -> VkCommandBuffer { return command_buffers[index]; }

	auto buffer_index() -> std::uint32_t
	{
		// Frame dependent buffer
		if (is_frame_dependent_executor) {
			return current;
		}

		// This buffer is owned by the swapchain, technically never called
		if (props.owned_by_swapchain) {
			return swapchain->get_current_frame();
		}

		// Immediate buffer.
		return 0;
	}

	void wait_indefinite() override;

	auto get_gpu_execution_time(uint32_t frame_index, uint32_t query_index = 0) const -> float override
	{
		if (query_index == UINT32_MAX || query_index / 2 >= timestamp_next_available_query / 2) {
			return 0.0F;
		}

		return execution_gpu_times[frame_index][query_index / 2];
	}

	auto get_pipeline_statistics(uint32_t frame_index) const -> const PipelineStatistics& override
	{
		return pipeline_statistics_query_results[frame_index];
	}

	auto has_stats() const -> bool override { return props.record_stats; }

private:
	void recreate_executor(bool should_clean = true);
	void create_query_pools();
	void record_stats();
	void destroy_executor();
	void create_base_structures();

	const Disarray::Device& device;
	const Disarray::Swapchain* swapchain;
	const Disarray::QueueFamilyIndex& indexes;
	bool is_frame_dependent_executor { false };

	std::uint32_t current { 0 };
	std::uint32_t image_count { 0 };
	VkCommandPool command_pool {};
	std::vector<VkCommandBuffer> command_buffers;
	VkCommandBuffer active { nullptr };
	std::vector<VkFence> fences;
	VkQueue graphics_queue {};

	std::uint32_t timestamp_query_count { 0 };
	uint32_t timestamp_next_available_query { 2 };
	std::vector<VkQueryPool> timestamp_query_pools;
	std::vector<VkQueryPool> pipeline_statistics_query_pools;
	std::vector<std::vector<uint64_t>> timestamp_query_results;
	std::vector<std::vector<float>> execution_gpu_times;

	std::uint32_t pipeline_query_count { 0 };
	std::vector<PipelineStatistics> pipeline_statistics_query_results;
};

struct VulkanImmediateCommandBuffer : public Disarray::IndependentCommandExecutor<VkCommandBuffer> {
	DISARRAY_MAKE_NONCOPYABLE(VulkanImmediateCommandBuffer)
public:
	using IndependentCommandExecutor<VkCommandBuffer>::IndependentCommandExecutor;
	~VulkanImmediateCommandBuffer() override {};

	[[nodiscard]] auto supply() const
	{
		const auto& exec = get_executor();

		return supply_cast<Vulkan::CommandExecutor>(exec);
	}
};

namespace {
	struct Deleter {
		void operator()(VulkanImmediateCommandBuffer* to_destroy)
		{
			to_destroy->submit_and_end();
			to_destroy->wait_indefinite();
			delete to_destroy;
		}
	};

} // namespace

inline auto construct_immediate(const Disarray::Device& device) -> Scope<VulkanImmediateCommandBuffer, Deleter>
{
	using T = VulkanImmediateCommandBuffer;
	Scope<T, Deleter> executor = make_scope<T, Deleter>(device);
	executor->begin();
	return executor;
}

} // namespace Disarray::Vulkan
