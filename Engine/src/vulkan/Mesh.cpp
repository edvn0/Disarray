#include "vulkan/Mesh.hpp"

#include "graphics/IndexBuffer.hpp"
#include "graphics/VertexBuffer.hpp"

#include <glm/glm.hpp>

namespace Disarray::Vulkan {

	Mesh::Mesh(Ref<Disarray::Device> dev, Ref<Disarray::Swapchain> swapchain, Ref<Disarray::PhysicalDevice> physical_device, const Disarray::MeshProperties& properties)
		: device(dev)
		, props(properties)
	{
		struct Vertex {
			glm::vec3 pos;
			glm::vec2 uv;
			glm::vec4 colour;
		};

		Vertex data[] = {
			{ {0.0, -0.5, 0.0}, {0, 1}, glm::vec4(1,0,0,1) },
			{ {0.5, 0.5, 0.0}, {1, 1}, glm::vec4(0,1,0,1) },
			{ {-0.5, 0.5, 0.0}, {-1, 1}, glm::vec4(0,0,0,1) }
		};

		vertices = Disarray::VertexBuffer::construct(device, swapchain, physical_device, {.data = data, .size = sizeof(Vertex) * 3,});

		std::uint32_t index_data[] = {
			0, 1, 2, 0
		};

		indices = Disarray::IndexBuffer::construct(device, swapchain, physical_device, {.data = index_data, .size = sizeof(std::uint32_t) * 4});
	}

	Mesh::~Mesh() { }

} // namespace Disarray::Vulkan