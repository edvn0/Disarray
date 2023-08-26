#include "core/Layer.hpp"

namespace Disarray {

void Layer::construct(App&, ThreadPool&) { }
void Layer::handle_swapchain_recreation(Swapchain&) { }
void Layer::on_event(Event&) { }
void Layer::interface() { }
void Layer::update(float) { }
void Layer::render() { }
void Layer::destruct() { }
bool Layer::is_interface_layer() const { return false; }

} // namespace Disarray
