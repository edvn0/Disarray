#pragma once

#include "core/Types.hpp"

namespace Disarray {

	class Renderer;

	class Layer {
	public:
		virtual ~Layer() = default;

		virtual void construct() = 0;
		virtual void handle_swapchain_recreation(Ref<Renderer>) = 0;

		virtual void update(float ts) = 0;
		virtual void update(float ts, Ref<Renderer>) = 0;

		virtual void destruct() = 0;
	};

} // namespace Disarray
