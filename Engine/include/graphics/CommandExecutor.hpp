#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <type_traits>

#include "core/CleanupAwaiter.hpp"
#include "core/DisarrayObject.hpp"
#include "core/Types.hpp"

namespace Disarray {

struct PipelineStatistics {
	std::uint64_t input_assembly_vertices { 0 };
	std::uint64_t input_assembly_primitives { 0 };
	std::uint64_t vertex_shader_invocations { 0 };
	std::uint64_t clipping_invocations { 0 };
	std::uint64_t clipping_primitives { 0 };
	std::uint64_t fragment_shader_invocations { 0 };
	std::uint64_t compute_shader_invocations { 0 };
};

struct CommandExecutorProperties {
	std::optional<std::uint32_t> count { std::nullopt };
	bool is_primary { true };
	bool owned_by_swapchain { false };
	bool record_stats { false };
};

class CommandExecutor : public ReferenceCountable {
	DISARRAY_OBJECT(CommandExecutor)
public:
	template <class T>
		requires(std::is_base_of_v<CommandExecutor, T>)
	static Ref<T> construct_as(Disarray::Device& device, Disarray::Swapchain& swapchain, const CommandExecutorProperties& props)
	{
		return cast_to<T>(CommandExecutor::construct(device, swapchain, props));
	}

	static Ref<CommandExecutor> construct(Disarray::Device&, Disarray::Swapchain&, const CommandExecutorProperties&);
	static Ref<CommandExecutor> construct(const Disarray::Device&, const Disarray::Swapchain&, const CommandExecutorProperties&);

	virtual void begin() = 0;
	virtual void end() = 0;
	virtual void submit_and_end() = 0;

	virtual float get_gpu_execution_time(uint32_t frame_index, uint32_t query_index = 0) const = 0;
	virtual const PipelineStatistics& get_pipeline_statistics(uint32_t frame_index) const = 0;

	virtual bool has_stats() const = 0;
};

class IndependentCommandExecutor : public Disarray::CommandExecutor {
public:
	virtual ~IndependentCommandExecutor() override = default;
	virtual void recreate(bool, const Extent&) override {};
	virtual void force_recreation() override {};

public:
	template <class T>
		requires(std::is_base_of_v<CommandExecutor, T>)
	static Ref<T> construct_as(Disarray::Device& device, const CommandExecutorProperties& props)
	{
		return cast_to<T>(IndependentCommandExecutor::construct(device, props));
	}

	static Scope<CommandExecutor> construct(Disarray::Device&, const CommandExecutorProperties&);

	virtual void begin() override = 0;
	virtual void end() override = 0;
	virtual void submit_and_end() override = 0;

	virtual float get_gpu_execution_time(uint32_t frame_index, uint32_t query_index = 0) const override = 0;
	virtual const PipelineStatistics& get_pipeline_statistics(uint32_t frame_index) const override = 0;

	virtual bool has_stats() const override = 0;
};

} // namespace Disarray
