#pragma once

namespace Disarray {

	class Layer {
	public:
		virtual ~Layer() = default;

		virtual void construct() = 0;
		virtual void update(float ts) = 0;
		virtual void destruct() = 0;
	};

} // namespace Disarray
