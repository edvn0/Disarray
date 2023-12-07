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
	Surface(Instance&, GLFWwindow*);
	~Surface() override;

	[[nodiscard]] auto supply() const -> VkSurfaceKHR override { return surface; }
	[[nodiscard]] auto supply() -> VkSurfaceKHR override { return surface; }

private:
	Instance& instance;
	VkSurfaceKHR surface { nullptr };
};
} // namespace Disarray::Vulkan
