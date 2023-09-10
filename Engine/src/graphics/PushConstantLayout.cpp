#include "DisarrayPCH.hpp"

#include "graphics/PushConstantLayout.hpp"

namespace Disarray {

PushConstantLayout::PushConstantLayout(const std::initializer_list<PushConstantRange>& ranges_input)
	: ranges(ranges_input)
{
}

auto PushConstantLayout::size() const -> std::size_t { return ranges.size(); }

auto PushConstantLayout::get_input_ranges() const -> const std::vector<PushConstantRange>& { return ranges; }

} // namespace Disarray
