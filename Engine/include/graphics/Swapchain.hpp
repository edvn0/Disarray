#pragma once

#include "ImageProperties.hpp"
#include "core/Types.hpp"

namespace Disarray {

	class Device;
	class PhysicalDevice;
	class Window;

	class Swapchain {
	public:
		static Ref<Swapchain> construct(Scope<Window>&, Ref<Device>, Ref<PhysicalDevice>, Ref<Swapchain> = nullptr);

		virtual std::uint32_t image_count() const = 0;
		virtual Extent get_extent() const = 0;

		virtual std::uint32_t get_current_frame() = 0;
		virtual std::uint32_t advance_frame() = 0;
		virtual std::uint32_t get_image_index() = 0;

		virtual bool prepare_frame() = 0;
		virtual void present() = 0;

		virtual bool needs_recreation() = 0;
		virtual void reset_recreation_status() = 0;

		virtual ~Swapchain() = default;
	};

} // namespace Disarray