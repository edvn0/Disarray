#pragma once

#include "core/Types.hpp"

namespace Disarray {

	class Instance {
	public:
		static Scope<Instance> construct();
		virtual ~Instance() = default;
	};

} // namespace Disarray
