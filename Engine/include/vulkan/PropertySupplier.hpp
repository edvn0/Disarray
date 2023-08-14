#pragma once

#include "core/Log.hpp"
#include "core/Types.hpp"
#include "vulkan/Structures.hpp"
#include "vulkan/Verify.hpp"

#include <stdexcept>
#include <vulkan/vulkan.h>

namespace Disarray::Vulkan {

template <class T> class PropertySupplier {
public:
	virtual ~PropertySupplier() = default;
	virtual T supply() const = 0;
	virtual T operator*() const { return supply(); }
};

} // namespace Disarray::Vulkan
