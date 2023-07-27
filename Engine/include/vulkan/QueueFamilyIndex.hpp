#pragma once

#include "core/Types.hpp"

#include <optional>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

namespace Disarray {
	class Surface;
	class PhysicalDevice;
} // namespace Disarray

namespace Disarray::Vulkan {

	class QueueFamilyIndex {
	public:
		explicit QueueFamilyIndex(Ref<Disarray::PhysicalDevice>, Ref<Disarray::Surface>);
		explicit QueueFamilyIndex(VkPhysicalDevice, Ref<Disarray::Surface>);

		bool is_complete() const { return graphics.has_value() && compute.has_value() && present.has_value(); }

		std::uint32_t get_graphics_family() const { return get_or_throw(graphics); };
		std::uint32_t get_compute_family() const { return get_or_throw(compute); };
		std::uint32_t get_transfer_family() const { return get_or_throw(transfer); };
		std::uint32_t get_present_family() const { return get_or_throw(present); };

		operator bool() const { return is_complete(); }

	private:
		template <typename T> std::uint32_t get_or_throw(T t) const
		{
			if (!t)
				throw std::runtime_error("Missing value");
			return *t;
		}

		std::optional<std::uint32_t> graphics { std::nullopt };
		std::optional<std::uint32_t> compute { std::nullopt };
		std::optional<std::uint32_t> transfer { std::nullopt };
		std::optional<std::uint32_t> present { std::nullopt };
	};

} // namespace Disarray::Vulkan
