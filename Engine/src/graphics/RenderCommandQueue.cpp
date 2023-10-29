#include "DisarrayPCH.hpp"

#include "graphics/RenderCommandQueue.hpp"

#include "core/Collections.hpp"
#include "core/Log.hpp"

namespace Disarray {

RenderCommandQueue::RenderCommandQueue(UsageBadge<Renderer>) { }

RenderCommandQueue::~RenderCommandQueue() = default;

void RenderCommandQueue::allocate(RenderCommandFunction&& fn) { storage_buffer[command_count++] = std::move(fn); }

void RenderCommandQueue::execute()
{
	Collections::parallel_for_each(storage_buffer, [](auto& func) {
		if (func) {
			func();
		}
	});
	command_count = 0;
	storage_buffer.fill({});
}

} // namespace Disarray
