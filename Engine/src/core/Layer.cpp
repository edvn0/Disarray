#include "core/Layer.hpp"

namespace Disarray {

	void Layer::construct(App&, Renderer&, ThreadPool&) { }
	void Layer::handle_swapchain_recreation(Renderer&) { }
	void Layer::interface() { }
	void Layer::update(float ts) { }
	void Layer::update(float ts, Renderer&) { }
	void Layer::destruct() { }
	bool Layer::is_interface_layer() const { return false; }

} // namespace Disarray
