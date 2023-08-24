#pragma once

namespace Disarray::Log {
std::string format(const char* const format, ...);
}

#if defined(IS_DEBUG) && defined(DEBUG_ALLOCATIONS)
#define VMA_DEBUG_LOG_FORMAT(fmt, ...)                                                                                                               \
	do {                                                                                                                                             \
		using namespace Disarray::Log;                                                                                                               \
		debug("VMA", format(fmt, __VA_ARGS__));                                                                                                      \
	} while (false)
#endif

#include <vk_mem_alloc.h>
