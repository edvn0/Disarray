#pragma once

#include "core/ReferenceCounted.hpp"
#include "core/ThreadPool.hpp"
#include "core/Types.hpp"
#include "core/events/Event.hpp"

namespace Disarray {

class App;
class Swapchain;

class Layer {
public:
	virtual ~Layer() = default;

	virtual void construct(App&);
	virtual void handle_swapchain_recreation(Swapchain&);
	virtual void on_event(Event&);
	virtual void interface();
	virtual void update(float time_step);
	virtual void render();
	virtual void destruct();
	[[nodiscard]] virtual auto is_interface_layer() const -> bool;
};

} // namespace Disarray
