#include "DisarrayPCH.hpp"

#include "graphics/PhysicalDevice.hpp"
#include "vulkan/Allocator.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Structures.hpp"
#include "vulkan/UniformBuffer.hpp"

#include <vulkan/vulkan.h>

namespace Disarray::Vulkan {

UniformBuffer::UniformBuffer(Disarray::Device& dev, const Disarray::BufferProperties& properties)
	: BaseBuffer(dev, BufferType::Uniform, properties)
{
}

} // namespace Disarray::Vulkan
