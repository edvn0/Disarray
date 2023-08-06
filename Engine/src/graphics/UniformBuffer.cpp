#include "DisarrayPCH.hpp"

#include "graphics/UniformBuffer.hpp"

#include "vulkan/UniformBuffer.hpp"

namespace Disarray {

	Ref<UniformBuffer> UniformBuffer::construct(Disarray::Device& device, Disarray::Swapchain& swapchain, const Disarray::BufferProperties& props)
	{
		return make_ref<Vulkan::UniformBuffer>(device, swapchain, props);
	}

} // namespace Disarray