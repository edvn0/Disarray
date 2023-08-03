#pragma once

#include "core/CleanupAwaiter.hpp"
#include "core/Types.hpp"

#include <optional>
#include <type_traits>

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
		template <class T>
			requires(std::is_base_of_v<CommandExecutor, T>)
		static Ref<T> construct_as(Disarray::Device& device, Disarray::Swapchain& swapchain, const CommandExecutorProperties& props)
		{
			return cast_to<T>(CommandExecutor::construct(device, swapchain, props));
		}

		static Ref<CommandExecutor> construct(Disarray::Device&, Disarray::Swapchain&, const CommandExecutorProperties&);
		static Ref<CommandExecutor> construct_from_swapchain(Disarray::Device&, Disarray::Swapchain&, CommandExecutorProperties);

		virtual void begin() = 0;
		virtual void end() = 0;
		virtual void submit_and_end() = 0;

		virtual void recreate(bool should_clean) = 0;
		virtual void force_recreation() = 0;
	};

} // namespace Disarray
