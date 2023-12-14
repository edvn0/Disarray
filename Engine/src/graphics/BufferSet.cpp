#include "DisarrayPCH.hpp"

#include "graphics/BufferSet.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/RendererProperties.hpp"

namespace Disarray::Detail {

auto get_current_frame(const Disarray::IGraphicsResource* graphics_resource) -> FrameIndex
{
	if (graphics_resource == nullptr) {
		return FrameIndex(0);
	}
	return graphics_resource->get_current_frame_index();
}

} // namespace Disarray::Detail
