#include "DisarrayPCH.hpp"

#include "graphics/Mesh.hpp"

#include "vulkan/Mesh.hpp"

namespace Disarray {

	Ref<Mesh> Mesh::construct(Disarray::Device& device, Disarray::Swapchain& swapchain, const Disarray::MeshProperties& props)
	{
		return make_ref<Vulkan::Mesh>(device, swapchain, props);
	}

} // namespace Disarray
