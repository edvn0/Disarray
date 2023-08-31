#include "DisarrayPCH.hpp"

#include "graphics/Pipeline.hpp"

#include "vulkan/Pipeline.hpp"

namespace Disarray {

Ref<Pipeline> Pipeline::construct(const Disarray::Device& device, Disarray::PipelineProperties props)
{
	return make_ref<Vulkan::Pipeline>(device, std::move(props));
}

} // namespace Disarray
