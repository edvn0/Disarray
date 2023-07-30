#include "graphics/PushContantLayout.hpp"

namespace Disarray {

	PushConstantLayout::PushConstantLayout(const std::initializer_list<PushConstantRange>& in)
		: ranges(in)
	{
	}

} // namespace Alabaster
