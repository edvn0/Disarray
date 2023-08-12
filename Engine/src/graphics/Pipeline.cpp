#include "DisarrayPCH.hpp"

#include "graphics/Pipeline.hpp"

#include "vulkan/Pipeline.hpp"

namespace Disarray {

	Ref<Pipeline> Pipeline::construct(const Disarray::Device& device, const Disarray::PipelineProperties& props)
	{
		return make_ref<Vulkan::Pipeline>(device, props);
	}

} // namespace Disarray
