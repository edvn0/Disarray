#pragma once

#include <vulkan/vulkan.h>

#include <stdexcept>

#include "core/Log.hpp"
#include "core/Types.hpp"
#include "vulkan/Structures.hpp"
#include "vulkan/Verify.hpp"

namespace Disarray::Vulkan {

template <class T> class PropertySupplier {
public:
	virtual ~PropertySupplier() = default;
	virtual T supply() const = 0;
	virtual T operator*() const { return supply(); }
};

} // namespace Disarray::Vulkan
