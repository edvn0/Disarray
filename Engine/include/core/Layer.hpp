#pragma once

#include "core/ReferenceCounted.hpp"
#include "core/Types.hpp"

namespace Disarray {

	class Renderer;
	class App;

	class Layer {
	public:
		virtual ~Layer() = default;

		virtual void construct(App&, Renderer&) = 0;
		virtual void handle_swapchain_recreation(Renderer&) = 0;

		virtual void interface() = 0;

		virtual void update(float ts) = 0;
		virtual void update(float ts, Renderer&) = 0;

		virtual void destruct() = 0;

		virtual bool is_interface_layer() const { return false; };
	};

	class Panel : public Layer {
	public:
		virtual ~Panel() = default;
	};

} // namespace Disarray
