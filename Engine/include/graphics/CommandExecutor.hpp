#pragma once

#include "core/CleanupAwaiter.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/Types.hpp"
#include "graphics/Device.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "graphics/Swapchain.hpp"

#include <optional>

namespace Disarray {

	struct CommandExecutorProperties {
		std::optional<std::uint32_t> count { std::nullopt };
		bool is_primary { true };
		bool owned_by_swapchain { false };
	};

	class CommandExecutor : public ReferenceCountable {
		DISARRAY_MAKE_REFERENCE_COUNTABLE(CommandExecutor)
	public:
		static Ref<CommandExecutor> construct(Disarray::Device&, Disarray::Swapchain&, const CommandExecutorProperties&);
		static Ref<CommandExecutor> construct_from_swapchain(Disarray::Device&, Disarray::Swapchain&, CommandExecutorProperties);

		virtual void begin() = 0;
		virtual void end() = 0;
		virtual void submit_and_end() = 0;

		virtual void force_recreation() = 0;
	};

	template <typename T> static decltype(auto) construct_immediate(Disarray::Device& device, Disarray::Swapchain& swapchain)
	{
		auto base_executor = CommandExecutor::construct(device, swapchain, { .count = 1, .owned_by_swapchain = false });
		auto executor = base_executor.as<T>();
		executor->begin();
		auto destructor = [&device](auto& command_executor) {
			command_executor->submit_and_end();
			wait_for_cleanup(device);
			command_executor.reset();
		};
		return std::make_pair(std::move(executor), std::move(destructor));
	}

} // namespace Disarray
