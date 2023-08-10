#pragma once

#include "core/ReferenceCounted.hpp"
#include "core/ThreadPool.hpp"
#include "core/Types.hpp"
#include "core/events/Event.hpp"

namespace Disarray {

	class Renderer;
	class App;

	class Layer {
	public:
		virtual ~Layer() = default;

		virtual void construct(App&, Renderer&, ThreadPool&);
		virtual void handle_swapchain_recreation(Renderer&);
		virtual void on_event(Event&);
		virtual void interface();
		virtual void update(float ts);
		virtual void update(float ts, Renderer&);
		virtual void destruct();
		virtual bool is_interface_layer() const;
	};

} // namespace Disarray
