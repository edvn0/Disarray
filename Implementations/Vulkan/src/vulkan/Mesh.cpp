#include "DisarrayPCH.hpp"

#include "vulkan/Mesh.hpp"

#include "graphics/IndexBuffer.hpp"
#include "graphics/ModelLoader.hpp"
#include "graphics/VertexBuffer.hpp"

namespace Disarray::Vulkan {

Mesh::~Mesh() = default;

Mesh::Mesh(const Disarray::Device& dev, const Disarray::MeshProperties& properties)
	: device(dev)
	, props(properties)
{
	ModelLoader loader { props.path, props.initial_rotation };
	vertices = Disarray::VertexBuffer::construct(device,
		{
			.data = loader.get_vertices().data(),
			.size = loader.get_vertices_size(),
			.count = loader.get_vertices_count(),
		});
	indices = Disarray::IndexBuffer::construct(device,
		{
			.data = loader.get_indices().data(),
			.size = loader.get_indices_size(),
			.count = loader.get_indices_count(),
		});
}

} // namespace Disarray::Vulkan
