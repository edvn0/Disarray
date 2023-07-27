#pragma once

namespace Disarray::Vulkan::Config {

#ifdef IS_DEBUG
	static constexpr bool is_debug = true;
#else
	static constexpr bool is_debug = false;
#endif

	static constexpr auto use_validation_layers = is_debug;

} // namespace Disarray::Vulkan::Config
