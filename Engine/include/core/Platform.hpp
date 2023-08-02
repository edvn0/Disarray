#pragma once

#ifdef WIN32

#ifndef DISARRAY_WINDOWS
#error How did you get here?
#endif

namespace Disarray {
	inline constexpr bool is_vulkan_supported = true;
}

#else

#ifndef DISARRAY_LINUX
#error How did you get here?
#endif

namespace Disarray {
	inline constexpr bool is_vulkan_supported = false;
}

#endif
