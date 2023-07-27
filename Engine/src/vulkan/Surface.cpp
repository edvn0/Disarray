#include "vulkan/Surface.hpp"

#include "core/Log.hpp"

#include <vulkan/vulkan.h>

namespace Disarray::Vulkan {

	Surface::Surface(GLFWwindow*) { }

	Surface::~Surface() { Log::debug("Surface destroyed."); }

} // namespace Disarray::Vulkan
