#include "graphics/PushConstantLayout.hpp"

namespace Disarray {

	PushConstantLayout::PushConstantLayout(const std::initializer_list<PushConstantRange>& in)
		: ranges(in)
	{
	}

} // namespace Disarray
