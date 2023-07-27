#include "graphics/Instance.hpp"

#include "vulkan/Instance.hpp"

namespace Disarray {

	Scope<Instance> Instance::construct() { return make_scope<Vulkan::Instance>(); }

} // namespace Disarray
