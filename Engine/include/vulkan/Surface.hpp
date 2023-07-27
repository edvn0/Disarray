#pragma once

#include "graphics/Surface.hpp"
#include "vulkan/PropertySupplier.hpp"

extern "C" {
struct GLFWwindow;
}

namespace Disarray::Vulkan {

	class Instance;

	class Surface : public Disarray::Surface, public PropertySupplier<VkSurfaceKHR> {
	public:
		Surface(Ref<Instance>, GLFWwindow*);
		~Surface() override;

		VkSurfaceKHR get() const override { return surface; }

	private:
		Ref<Instance> instance;
		VkSurfaceKHR surface;
	};
} // namespace Disarray::Vulkan
