#pragma once

#include "core/Types.hpp"
#include <optional>

namespace Disarray {

	struct CommandExecutorProperties {
		std::optional<std::uint32_t> count {std::nullopt};
		bool is_primary {true};
		bool owned_by_swapchain {false};
	};

	class Device;
	class PhysicalDevice;
	class Surface;
	class Swapchain;

	class CommandExecutor {
	public:
		virtual ~CommandExecutor() = default;
		static Ref<CommandExecutor> construct(Ref<Disarray::Device>, Ref<Disarray::PhysicalDevice>, Ref<Disarray::Swapchain>, Ref<Disarray::Surface>, const CommandExecutorProperties&);
		static Ref<CommandExecutor> construct_from_swapchain(Ref<Disarray::Device>, Ref<Disarray::PhysicalDevice>, Ref<Disarray::Swapchain>, Ref<Disarray::Surface>, CommandExecutorProperties);

		virtual void begin() = 0;
		virtual void end() = 0;
		virtual void submit_and_end() = 0;

		virtual void force_recreation() = 0;
	};

} // namespace Disarray