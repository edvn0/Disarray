#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <type_traits>

#include "core/CleanupAwaiter.hpp"
#include "core/DisarrayObject.hpp"
#include "core/Types.hpp"
#include "graphics/Device.hpp"

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
	DISARRAY_OBJECT_PROPS(CommandExecutor, CommandExecutorProperties)
public:
	virtual void begin() = 0;
	virtual void end() = 0;
	virtual void submit_and_end() = 0;

	virtual void wait_indefinite() = 0;

	virtual auto get_gpu_execution_time(std::uint32_t frame_index, std::uint32_t query_index = 0) const -> float = 0;
	virtual auto get_pipeline_statistics(std::uint32_t frame_index) const -> const PipelineStatistics& = 0;

	virtual auto has_stats() const -> bool = 0;

	static auto construct(const Disarray::Device&, const Disarray::Swapchain*, CommandExecutorProperties) -> Ref<Disarray::CommandExecutor>;
};

template <class T> class IndependentCommandExecutor {
	DISARRAY_MAKE_NONCOPYABLE(IndependentCommandExecutor)
public:
	IndependentCommandExecutor(const Disarray::Device& device)
	{
		executor = CommandExecutor::construct_scoped(device,
			{
				.count = 1,
				.is_primary = true,
				.owned_by_swapchain = false,
				.record_stats = false,
			});
	}
	virtual ~IndependentCommandExecutor() = default;

	void begin() { executor->begin(); }
	void submit_and_end() { executor->submit_and_end(); };

	[[nodiscard]] static auto buffer_index() -> std::uint32_t { return 0; }

	void wait_indefinite() { executor->wait_indefinite(); }

protected:
	auto get_executor() const -> const auto& { return *executor; }

private:
	Scope<Disarray::CommandExecutor> executor { nullptr };
};

} // namespace Disarray
