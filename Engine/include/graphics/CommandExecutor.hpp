#pragma once

#include "core/CleanupAwaiter.hpp"
#include "core/Types.hpp"

#include <optional>

namespace Disarray {

	struct CommandExecutorProperties {
		std::optional<std::uint32_t> count { std::nullopt };
		bool is_primary { true };
		bool owned_by_swapchain { false };
	};

	class Device;
	class PhysicalDevice;
	class Surface;
	class Swapchain;
	class QueueFamilyIndex;

	class CommandExecutor {
	public:
		virtual ~CommandExecutor() = default;
		static Ref<CommandExecutor> construct(
			Ref<Disarray::Device>, Ref<Disarray::Swapchain>, Ref<Disarray::QueueFamilyIndex>, const CommandExecutorProperties&);
		static Ref<CommandExecutor> construct_from_swapchain(
			Ref<Disarray::Device>, Ref<Disarray::Swapchain>, Ref<Disarray::QueueFamilyIndex>, CommandExecutorProperties);

		virtual void begin() = 0;
		virtual void end() = 0;
		virtual void submit_and_end() = 0;

		virtual void force_recreation() = 0;
	};

	template <typename T>
	static decltype(auto) construct_immediate(Ref<Disarray::Device> device,
		Ref<Disarray::Swapchain> swapchain, Ref<Disarray::QueueFamilyIndex> queue_family_index)
	{
		auto executor = cast_to<T>(CommandExecutor::construct(device, swapchain, queue_family_index, { .count = 1, .owned_by_swapchain = false }));
		executor->begin();
		auto destructor = [&device](auto& command_executor) {
			command_executor->submit_and_end();
			wait_for_cleanup(device);
			command_executor.reset();
		};
		return std::make_pair(std::move(executor), std::move(destructor));
	}

} // namespace Disarray