#pragma once

#include <cstddef>
#include <cstdint>

namespace Disarray {

enum class BufferType : std::uint8_t { Vertex, Index, Uniform, Storage };

struct BufferProperties {
	const void* data {};
	std::size_t size {};
	std::size_t count { 1 };
	bool always_mapped { false };
};

} // namespace Disarray
