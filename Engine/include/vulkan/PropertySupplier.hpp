#pragma once

#include "core/Log.hpp"
#include "core/Types.hpp"
#include "vulkan/Verify.hpp"

#include <stdexcept>
#include <vulkan/vulkan.h>

namespace Disarray::Vulkan {

	template <class T> class PropertySupplier {
	public:
		virtual T supply() const { throw std::runtime_error("Not implemented!"); };
		virtual T operator*() const { return supply(); }
	};

} // namespace Disarray::Vulkan
