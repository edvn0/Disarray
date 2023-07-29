#include "vulkan/Mesh.hpp"

#include "graphics/IndexBuffer.hpp"
#include "graphics/ModelLoader.hpp"
#include "graphics/VertexBuffer.hpp"

#include <glm/glm.hpp>

namespace Disarray::Vulkan {

	Mesh::Mesh(Ref<Disarray::Device> dev, Ref<Disarray::Swapchain> swapchain, Ref<Disarray::PhysicalDevice> physical_device, const Disarray::MeshProperties& properties)
		: device(dev)
		, props(properties)
	{
		ModelLoader loader {props.path};
		vertices = Disarray::VertexBuffer::construct(device, swapchain, physical_device, {.data = loader.get_vertices().data(), .size = loader.get_vertices_size()});
		indices = Disarray::IndexBuffer::construct(device, swapchain, physical_device, {.data = loader.get_indices().data(), .size = loader.get_indices_size()});
	}

	Mesh::~Mesh() { }

} // namespace Disarray::Vulkan