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
	virtual auto supply() const -> T = 0;
	virtual auto supply() -> T = 0;
	virtual auto operator*() const -> T { return supply(); }
	virtual auto operator*() -> T  { return supply(); }
};

} // namespace Disarray::Vulkan
