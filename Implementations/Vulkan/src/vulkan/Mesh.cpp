#include "DisarrayPCH.hpp"

#include "graphics/Mesh.hpp"

#include <algorithm>
#include <cstdint>
#include <exception>
#include <future>
#include <iostream>
#include <mutex>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "core/Log.hpp"
#include "core/ThreadPool.hpp"
#include "core/Types.hpp"
#include "core/exceptions/GeneralExceptions.hpp"
#include "graphics/BufferProperties.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/ModelLoader.hpp"
#include "graphics/ModelVertex.hpp"
#include "graphics/VertexBuffer.hpp"
#include "graphics/model_loaders/AssimpModelLoader.hpp"
#include "vulkan/Mesh.hpp"

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
		ModelLoader loader { make_scope<AssimpModelLoader>(props.initial_rotation), props.path };
		mesh_textures = loader.construct_textures(device);
		for (const auto& mesh_data = loader.get_mesh_data(); const auto& [key, submesh] : mesh_data) {
			auto vertex_buffer = VertexBuffer::construct_scoped(device,
				{
					.data = submesh.data<ModelVertex>(),
					.size = submesh.size<ModelVertex>(),
					.count = submesh.count<ModelVertex>(),
				});
			auto index_buffer = IndexBuffer::construct_scoped(device,
				{
					.data = submesh.data<std::uint32_t>(),
					.size = submesh.size<std::uint32_t>(),
					.count = submesh.count<std::uint32_t>(),
				});
			auto substructure = make_scope<MeshSubstructure>(std::move(vertex_buffer), std::move(index_buffer));
			submeshes.try_emplace(key, std::move(substructure));
		}

		mesh_name = props.path.filename().replace_extension().string();
	} catch (const CouldNotLoadModelException& exc) {
		Log::error("Mesh", "Model could not be loaded: {}", exc.what());
	}
}

auto Mesh::get_indices() const -> Disarray::IndexBuffer& { return *submeshes.at(mesh_name)->indices; }

auto Mesh::get_vertices() const -> Disarray::VertexBuffer& { return *submeshes.at(mesh_name)->vertices; }

void Mesh::force_recreation() { load_and_initialise_model(); }

auto Mesh::get_textures() const -> const RefVector<Disarray::Texture>& { return mesh_textures; }
auto Mesh::get_submeshes() const -> const Collections::ScopedStringMap<Disarray::MeshSubstructure>& { return submeshes; }

auto Mesh::has_children() const -> bool { return submeshes.size() > 1ULL; }

struct MeshConstructor {
	auto operator()(const auto& device, MeshProperties props) const { return Mesh::construct(device, std::move(props)); }
};

auto Mesh::construct_deferred(const Disarray::Device& device, MeshProperties properties) -> std::future<Ref<Disarray::Mesh>>
{
	return std::async(std::launch::async, MeshConstructor {}, std::cref(device), std::move(properties));
}

} // namespace Disarray::Vulkan
