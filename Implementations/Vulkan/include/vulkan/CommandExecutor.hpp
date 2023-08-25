#pragma once

#include <tuple>
#include <type_traits>
#include <vector>

#include "core/ReferenceCounted.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Device.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "graphics/QueueFamilyIndex.hpp"
#include "graphics/Surface.hpp"
#include "vulkan/PropertySupplier.hpp"
#include "vulkan/QueueFamilyIndex.hpp"
#include "vulkan/Swapchain.hpp"

namespace Disarray::Vulkan {

class CommandExecutor : public Disarray::CommandExecutor, public PropertySupplier<VkCommandBuffer> {
public:
	CommandExecutor(Disarray::Device&, Disarray::Swapchain&, const Disarray::CommandExecutorProperties&);
	CommandExecutor(const Disarray::Device&, const Disarray::Swapchain&, const Disarray::CommandExecutorProperties&);
	~CommandExecutor() override;

	void begin() override;
	void end() override;
	void submit_and_end() override;

	void begin(VkCommandBufferBeginInfo);

	void force_recreation() override { recreate_executor(); }
	void recreate(bool should_clean, const Extent&) override { return recreate_executor(should_clean); }

	VkCommandBuffer supply() const override { return active; }
	VkCommandBuffer get_buffer(std::uint32_t index) { return command_buffers[index]; }

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

	const PipelineStatistics& get_pipeline_statistics(uint32_t frame_index) const override { return pipeline_statistics_query_results[frame_index]; }

	bool has_stats() const override { return props.record_stats; }

private:
	void recreate_executor(bool should_clean = true);
	void create_query_pools();
	void record_stats();
	void destroy_executor();
	void create_base_structures();

	const Disarray::Device& device;
	const Disarray::Swapchain& swapchain;
	const Disarray::QueueFamilyIndex& indexes;
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

class IndependentCommandExecutor : public Disarray::IndependentCommandExecutor, public PropertySupplier<VkCommandBuffer> {
public:
	IndependentCommandExecutor(const Disarray::Device&, const Disarray::CommandExecutorProperties&);
	~IndependentCommandExecutor() override;

	void begin() override;
	void end() override;
	void submit_and_end() override;

	void begin(VkCommandBufferBeginInfo);

	void force_recreation() override { recreate_executor(); }
	void recreate(bool should_clean, const Extent&) override { return recreate_executor(should_clean); }

	VkCommandBuffer supply() const override { return active; }

	auto buffer_index() -> std::uint32_t
	{
		// Frame dependent buffer
		if (is_frame_dependent_executor) {
			return current;
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

	const PipelineStatistics& get_pipeline_statistics(uint32_t frame_index) const override { return pipeline_statistics_query_results[frame_index]; }

	bool has_stats() const override { return props.record_stats; }

private:
	void recreate_executor(bool should_clean = true);
	void create_query_pools();
	void record_stats();
	void destroy_executor();
	void create_base_structures();

	const Disarray::Device& device;
	const Disarray::QueueFamilyIndex& indexes;
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

namespace {
	void submit_and_delete_executor(Vulkan::IndependentCommandExecutor* to_destroy)
	{
		to_destroy->submit_and_end();
		to_destroy->wait_indefinite();
		delete to_destroy;
	}
} // namespace

constexpr auto deleter = [](Vulkan::IndependentCommandExecutor* l) { submit_and_delete_executor(l); };
using ImmediateExecutor = std::unique_ptr<Vulkan::IndependentCommandExecutor, decltype(deleter)>;
inline ImmediateExecutor construct_immediate(const Disarray::Device& device)
{
	static constexpr Disarray::CommandExecutorProperties props { .count = 1, .owned_by_swapchain = false };
	ImmediateExecutor executor { new IndependentCommandExecutor { device, props } };
	executor->begin();
	return executor;
}

} // namespace Disarray::Vulkan