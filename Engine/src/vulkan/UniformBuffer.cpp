#include "DisarrayPCH.hpp"

#include "vulkan/UniformBuffer.hpp"

#include "graphics/PhysicalDevice.hpp"
#include "vulkan/Allocator.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Structures.hpp"

#include <vulkan/vulkan.h>

namespace Disarray::Vulkan {

	UniformBuffer::UniformBuffer(Disarray::Device& dev, Disarray::Swapchain& swapchain, const Disarray::BufferProperties& properties)
		: BaseBuffer(dev, swapchain, BufferType::Uniform, properties)
	{
	}

} // namespace Disarray::Vulkan
