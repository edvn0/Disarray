#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Disarray {

enum class PushConstantKind : std::uint8_t { Vertex = 1 << 0, Fragment = 1 << 1, Both = 1 << 2 };

constexpr auto operator|(PushConstantKind left, PushConstantKind right) -> PushConstantKind
{
	return static_cast<PushConstantKind>(static_cast<std::uint32_t>(left) | static_cast<std::uint32_t>(right));
}

constexpr auto operator&(PushConstantKind left, PushConstantKind right) -> PushConstantKind
{
	return static_cast<PushConstantKind>(static_cast<std::uint32_t>(left) & static_cast<std::uint32_t>(right));
}

struct PushConstantRange {

	template <std::integral SizeType>
	constexpr explicit(false) PushConstantRange(PushConstantKind in_flags, SizeType in_size, SizeType offset = SizeType { 0 })
		: size(static_cast<std::uint32_t>(in_size))
		, flags(in_flags)
		, offset(static_cast<std::uint32_t>(offset))
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
	PushConstantLayout(const std::initializer_list<PushConstantRange>& ranges_in);
	PushConstantLayout(std::vector<PushConstantRange>&& ranges_in);
	[[nodiscard]] auto get_input_ranges() const -> const std::vector<PushConstantRange>&;
	[[nodiscard]] auto size() const -> std::size_t;

	[[nodiscard]] auto empty() const -> bool { return ranges.empty(); }

private:
	std::vector<PushConstantRange> ranges;
};
} // namespace Disarray
