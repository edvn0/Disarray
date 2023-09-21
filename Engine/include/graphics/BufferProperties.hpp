#pragma once

#include <cstddef>

namespace Disarray {

enum class BufferType : std::uint8_t { Vertex, Index, Uniform };

struct BufferProperties {
	const void* data {};
	std::size_t size {};
	std::size_t count { 1 };
	bool always_mapped { true };
};

} // namespace Disarray
