#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Disarray {

enum class PushConstantKind { Vertex = 1 << 0, Fragment = 1 << 1, Both = 1 << 2 };

constexpr PushConstantKind operator|(PushConstantKind a, PushConstantKind b)
{
	return static_cast<PushConstantKind>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
}

constexpr PushConstantKind operator&(PushConstantKind a, PushConstantKind b)
{
	return static_cast<PushConstantKind>(static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
}

struct PushConstantRange {

	template <std::integral SizeType>
	constexpr explicit(false) PushConstantRange(PushConstantKind in_flags, SizeType in_size)
		: size(static_cast<std::uint32_t>(in_size))
		, flags(in_flags)
	{
	}

	constexpr PushConstantRange()
		: size(0)
		, flags(PushConstantKind::Both)
	{
	}

	std::uint32_t size;
	PushConstantKind flags;
	std::uint32_t offset { 0 };
};

struct PushConstantLayout {
	PushConstantLayout(const std::initializer_list<PushConstantRange>& in);
	const auto& get_input_ranges() const { return ranges; }
	auto size() const { return ranges.size(); }

private:
	std::vector<PushConstantRange> ranges;
};
} // namespace Disarray
