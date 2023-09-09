#include "DisarrayPCH.hpp"

#include "graphics/Pipeline.hpp"

#include "vulkan/Pipeline.hpp"

namespace Disarray {

auto Pipeline::construct(const Disarray::Device& device, Disarray::PipelineProperties properties) -> Ref<Disarray::Pipeline>
{
	return make_ref<Vulkan::Pipeline>(device, std::move(properties));
}

} // namespace Disarray
