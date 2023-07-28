#pragma once

#include "graphics/Pipeline.hpp"

#include "vulkan/Pipeline.hpp"

namespace Disarray {

	Ref<Pipeline> Pipeline::construct(Ref<Disarray::Device> device, Ref<Disarray::Swapchain> swapchain, const Disarray::PipelineProperties& props)
	{
		return make_ref<Vulkan::Pipeline>(device,swapchain, props);
	}

}