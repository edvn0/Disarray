#include "DisarrayPCH.hpp"

#include "core/Log.hpp"
#include "core/Types.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Config.hpp"
#include "vulkan/DebugMarker.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/PhysicalDevice.hpp"
#include "vulkan/Swapchain.hpp"
#include "vulkan/Verify.hpp"

#include <core/Ensure.hpp>
#include <util/FormattingUtilities.hpp>
#include <vulkan/vulkan.h>

namespace Disarray::Vulkan {

// Random tag data
struct DemoTag {
	const char name[17] = "debug marker tag";
} demo;

CommandExecutor::CommandExecutor(Disarray::Device& dev, Disarray::Swapchain& sc, const Disarray::CommandExecutorProperties& properties)
	: device(dev)
	, swapchain(sc)
	, indexes(device.get_physical_device().get_queue_family_indexes())
	, props(properties)
	, is_frame_dependent_executor(properties.count.has_value() && *properties.count > 1 && !properties.owned_by_swapchain)
{
	recreate_executor(false);
}

CommandExecutor::CommandExecutor(const Disarray::Device& dev, const Disarray::Swapchain& sc, const Disarray::CommandExecutorProperties& properties)
	: device(dev)
	, swapchain(sc)
	, indexes(device.get_physical_device().get_queue_family_indexes())
	, props(properties)
	, is_frame_dependent_executor(properties.count.has_value() && *properties.count > 1 && !properties.owned_by_swapchain)
{
	recreate_executor(false);
}

CommandExecutor::~CommandExecutor() { destroy_executor(); }

void CommandExecutor::recreate_executor(bool should_clean)
{
	if (props.owned_by_swapchain)
		return;

	if (should_clean) {
		destroy_executor();
	}

	create_base_structures();
	create_query_pools();
}

void CommandExecutor::create_query_pools()
{
	if (!props.record_stats)
		return;

	const auto& vk_device = supply_cast<Vulkan::Device>(device);

	auto query_pool_create_info = vk_structures<VkQueryPoolCreateInfo> {}();

	// Timestamp queries
	const uint32_t max_user_queries = 16;
	timestamp_query_count = 2 + 2 * max_user_queries;

	query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
	query_pool_create_info.queryCount = timestamp_query_count;
	timestamp_query_pools.resize(image_count);
	for (auto& timestamp_query_pool : timestamp_query_pools) {
		verify(vkCreateQueryPool(vk_device, &query_pool_create_info, nullptr, &timestamp_query_pool));
	}

	timestamp_query_results.resize(image_count);
	for (auto& timestamp_query_result : timestamp_query_results)
		timestamp_query_result.resize(timestamp_query_count);

	execution_gpu_times.resize(image_count);
	for (auto& execution_gpu_time : execution_gpu_times)
		execution_gpu_time.resize(timestamp_query_count / 2);

	// Pipeline statistics queries
	pipeline_query_count = 7;
	query_pool_create_info.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
	query_pool_create_info.queryCount = pipeline_query_count;
	query_pool_create_info.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT
		| VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT | VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT
		| VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT | VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT
		| VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT | VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;

	pipeline_statistics_query_pools.resize(image_count);
	for (auto& pipeline_statistics_query_pool : pipeline_statistics_query_pools)
		verify(vkCreateQueryPool(vk_device, &query_pool_create_info, nullptr, &pipeline_statistics_query_pool));

	pipeline_statistics_query_results.resize(image_count);
}

void CommandExecutor::destroy_executor()
{
	if (props.owned_by_swapchain)
		return;

	const auto& vk_device = supply_cast<Vulkan::Device>(device);

	vkDestroyCommandPool(vk_device, command_pool, nullptr);
	if (props.record_stats) {
		for (auto& pool : timestamp_query_pools) {
			vkDestroyQueryPool(vk_device, pool, nullptr);
		}
		for (auto& pool : pipeline_statistics_query_pools) {
			vkDestroyQueryPool(vk_device, pool, nullptr);
		}
	}
	for (auto& fence : fences)
		vkDestroyFence(vk_device, fence, nullptr);
}

void CommandExecutor::create_base_structures()
{
	const auto vk_device = supply_cast<Vulkan::Device>(device);

	VkCommandPoolCreateInfo pool_info {};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool_info.queueFamilyIndex = indexes.get_graphics_family();

	verify(vkCreateCommandPool(vk_device, &pool_info, nullptr, &command_pool));

	auto count = props.count ? *props.count : swapchain.image_count();
	if (count > Config::max_frames_in_flight)
		count = Config::max_frames_in_flight;
	image_count = count;

	command_buffers.resize(image_count);
	VkCommandBufferAllocateInfo alloc_info {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = command_pool;
	alloc_info.level = props.is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	alloc_info.commandBufferCount = image_count;
	verify(vkAllocateCommandBuffers(vk_device, &alloc_info, command_buffers.data()));

	for (auto& buffer : command_buffers) {
		DebugMarker::set_object_tag(vk_device, buffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, 0, sizeof(demo), &demo);
		DebugMarker::set_object_name(vk_device, buffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, "Submit");
	}

	graphics_queue = cast_to<Vulkan::Device>(device).get_graphics_queue();

	fences.resize(image_count);

	VkFenceCreateInfo fence_create_info {};
	fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < image_count; i++) {
		verify(vkCreateFence(vk_device, &fence_create_info, nullptr, &fences[i]));
	}
}

void CommandExecutor::wait_indefinite() { vkDeviceWaitIdle(supply_cast<Vulkan::Device>(device)); }

void CommandExecutor::begin()
{
	VkCommandBufferBeginInfo begin_info {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = 0; // Optional
	begin_info.pInheritanceInfo = nullptr; // Optional

	if (props.owned_by_swapchain) {
		active = cast_to<Vulkan::Swapchain>(swapchain).get_drawbuffer();
	} else {
		active = command_buffers[buffer_index()];
	}
	verify(vkBeginCommandBuffer(active, &begin_info));

	record_stats();
}

void CommandExecutor::begin(VkCommandBufferBeginInfo begin_info)
{
	if (props.owned_by_swapchain) {
		active = cast_to<Vulkan::Swapchain>(swapchain).get_drawbuffer();
	} else {
		active = command_buffers[buffer_index()];
	}
	verify(vkBeginCommandBuffer(active, &begin_info));

	record_stats();
}

void CommandExecutor::end()
{
	if (props.record_stats) {
		vkCmdWriteTimestamp(active, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, timestamp_query_pools[buffer_index()], 1);
		vkCmdEndQuery(active, pipeline_statistics_query_pools[buffer_index()], 0);
	}

	vkEndCommandBuffer(active);
}

void CommandExecutor::submit_and_end()
{
	end();

	if (props.owned_by_swapchain) {
		active = nullptr;
		return;
	}

	const auto& vk_device = supply_cast<Vulkan::Device>(device);

	VkSubmitInfo submit_info {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &active;
	verify(vkWaitForFences(vk_device, 1, &fences[buffer_index()], VK_TRUE, UINT64_MAX));
	verify(vkResetFences(vk_device, 1, &fences[buffer_index()]));

	DebugMarker::begin_region(active, "Submit", { 0, 1, 1, 1 });
	verify(vkQueueSubmit(graphics_queue, 1, &submit_info, fences[buffer_index()]));
	DebugMarker::end_region(active);

	verify(vkWaitForFences(vk_device, 1, &fences[buffer_index()], VK_TRUE, UINT64_MAX));

	if (props.record_stats) {
		// Retrieve timestamp query results
		vkGetQueryPoolResults(vk_device, timestamp_query_pools[buffer_index()], 0, timestamp_next_available_query,
			timestamp_next_available_query * sizeof(uint64_t), timestamp_query_results[buffer_index()].data(), sizeof(uint64_t),
			VK_QUERY_RESULT_64_BIT);

		for (uint32_t i = 0; i < timestamp_next_available_query; i += 2) {
			uint64_t start_time = timestamp_query_results[buffer_index()][i];
			uint64_t end_time = timestamp_query_results[buffer_index()][i + 1];
			float ns_time = end_time > start_time
				? (end_time - start_time) * cast_to<Vulkan::PhysicalDevice>(device.get_physical_device()).get_limits().timestampPeriod
				: 0.0f;
			execution_gpu_times[buffer_index()][i / 2] = ns_time * 0.000001f; // Time in ms
		}

		// Retrieve pipeline stats results
		vkGetQueryPoolResults(vk_device, pipeline_statistics_query_pools[buffer_index()], 0, 1, sizeof(PipelineStatistics),
			&pipeline_statistics_query_results[buffer_index()], sizeof(uint64_t), VK_QUERY_RESULT_64_BIT);
	}

	current = (current + 1) % image_count;
}

void CommandExecutor::record_stats()
{
	if (props.record_stats) {
		// Timestamp query
		vkCmdResetQueryPool(active, timestamp_query_pools[buffer_index()], 0, timestamp_query_count);
		vkCmdWriteTimestamp(active, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, timestamp_query_pools[buffer_index()], 0);

		// Pipeline stats query
		vkCmdResetQueryPool(active, pipeline_statistics_query_pools[buffer_index()], 0, pipeline_query_count);
		vkCmdBeginQuery(active, pipeline_statistics_query_pools[buffer_index()], 0, 0);
	}
}

} // namespace Disarray::Vulkan
