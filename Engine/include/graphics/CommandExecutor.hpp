#pragma once

#include "core/CleanupAwaiter.hpp"
#include "core/Types.hpp"

#include <optional>

namespace Disarray {

	struct CommandExecutorProperties {
		std::optional<std::uint32_t> count { std::nullopt };
		bool is_primary { true };
		bool owned_by_swapchain { false };
		bool record_stats { false };
	};

	class CommandExecutor : public ReferenceCountable {
		DISARRAY_MAKE_REFERENCE_COUNTABLE(CommandExecutor)
	public:
		static Ref<CommandExecutor> construct(Disarray::Device&, Disarray::Swapchain&, const CommandExecutorProperties&);
		static Ref<CommandExecutor> construct_from_swapchain(Disarray::Device&, Disarray::Swapchain&, CommandExecutorProperties);

		virtual void begin() = 0;
		virtual void end() = 0;
		virtual void submit_and_end() = 0;

		virtual void recreate(bool should_clean) = 0;
		virtual void force_recreation() = 0;
	};

} // namespace Disarray
