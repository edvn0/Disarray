#pragma once

#include <cstddef>
#include <cstdint>

namespace Disarray {

enum class BufferType : std::uint8_t {
	Vertex = 0,
	Index = 1,
	Uniform = 2,
	Storage = 4,
};
constexpr auto operator|(BufferType left, BufferType right)
{
	return static_cast<BufferType>(static_cast<std::uint8_t>(left) | static_cast<std::uint8_t>(right));
}
constexpr auto operator&(BufferType left, BufferType right)
{
	return static_cast<BufferType>(static_cast<std::uint8_t>(left) & static_cast<std::uint8_t>(right));
}

struct BufferProperties {
	const void* data {};
	std::size_t size {};
	std::size_t count { 1 };
	std::uint32_t binding { 0 };
	bool always_mapped { false };
};

} // namespace Disarray
