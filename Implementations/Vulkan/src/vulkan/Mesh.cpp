#include "DisarrayPCH.hpp"

#include "vulkan/Mesh.hpp"

#include <exception>
#include <utility>

#include "core/Log.hpp"
#include "core/exceptions/GeneralExceptions.hpp"
#include "graphics/BufferProperties.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/ModelLoader.hpp"
#include "graphics/VertexBuffer.hpp"
#include "graphics/model_loaders/AssimpModelLoader.hpp"

namespace Disarray::Vulkan {

Mesh::~Mesh() = default;

Mesh::Mesh(const Disarray::Device& dev, Disarray::MeshProperties properties)
	: Disarray::Mesh(std::move(properties))
	, device(dev)
{
	load_and_initialise_model();
}

void Mesh::load_and_initialise_model()
{
	try {
		ModelLoader loader { make_scope<AssimpModelLoader>(), props.path };
		loader.construct_textures(device);

		submeshes = loader.get_mesh_data();

		const auto& name = props.path.filename().replace_extension().string();

		vertices = Disarray::VertexBuffer::construct_scoped(device,
			BufferProperties {
				.data = loader.get_vertices(name).data(),
				.size = loader.get_vertices_size(name),
				.count = loader.get_vertices_count(name),
			});
		indices = Disarray::IndexBuffer::construct_scoped(device,
			BufferProperties {
				.data = loader.get_indices(name).data(),
				.size = loader.get_indices_size(name),
				.count = loader.get_indices_count(name),
			});

	} catch (const CouldNotLoadModelException& exc) {
		Log::error("Mesh", "Model could not be loaded: {}", exc.what());
	}
}

void Mesh::force_recreation() { load_and_initialise_model(); }

auto Mesh::get_submeshes() const -> std::vector<Scope<Disarray::Mesh>> { return {}; }

auto Mesh::has_children() const -> bool { return false; }

} // namespace Disarray::Vulkan
