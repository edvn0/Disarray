#include "graphics/Mesh.hpp"

#include "vulkan/Mesh.hpp"

namespace Disarray {

	Ref<Mesh> Mesh::construct(Ref<Disarray::Device> device, Ref<Disarray::Swapchain> swapchain, Ref<Disarray::PhysicalDevice> physical_device,  const Disarray::MeshProperties& props)
	{
		return make_ref<Vulkan::Mesh>(device, swapchain, physical_device, props);
	}

}