#include "DisarrayPCH.hpp"

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
#include "graphics/Mesh.hpp"
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

Mesh::Mesh(const Disarray::Device& dev, Scope<Disarray::VertexBuffer> vertices, Scope<Disarray::IndexBuffer> indices,
	const std::vector<Ref<Disarray::Texture>>& textures)
	: device(dev)
	, vertex_buffer(std::move(vertices))
	, index_buffer(std::move(indices))
	, mesh_textures(textures)
{
}

void Mesh::load_and_initialise_model(const ImportedMesh& imported)
{
	if (imported.size() == 1) {
		const auto& kv = *imported.begin();
		auto&& [key, loaded_submesh] = kv;
		vertex_buffer = VertexBuffer::construct_scoped(device,
			{
				.data = loaded_submesh.data<ModelVertex>(),
				.size = loaded_submesh.size<ModelVertex>(),
				.count = loaded_submesh.count<ModelVertex>(),
			});
		index_buffer = IndexBuffer::construct_scoped(device,
			{
				.data = loaded_submesh.data<std::uint32_t>(),
				.size = loaded_submesh.size<std::uint32_t>(),
				.count = loaded_submesh.count<std::uint32_t>(),
			});
		aabb = loaded_submesh.aabb;
		mesh_textures = loaded_submesh.textures;

		return;
	}

	for (const auto& mesh_data = imported; const auto& [key, loaded_submesh] : mesh_data) {
		auto vb = VertexBuffer::construct_scoped(device,
			{
				.data = loaded_submesh.data<ModelVertex>(),
				.size = loaded_submesh.size<ModelVertex>(),
				.count = loaded_submesh.count<ModelVertex>(),
			});
		auto ib = IndexBuffer::construct_scoped(device,
			{
				.data = loaded_submesh.data<std::uint32_t>(),
				.size = loaded_submesh.size<std::uint32_t>(),
				.count = loaded_submesh.count<std::uint32_t>(),
			});

		auto submesh = Scope<Vulkan::Mesh> { new Vulkan::Mesh { device, std::move(vb), std::move(ib), loaded_submesh.textures } };
		submesh->aabb = loaded_submesh.aabb;
		submeshes.try_emplace(key, std::move(submesh));
	}
}

void Mesh::load_and_initialise_model()
{
	mesh_name = props.path.filename().replace_extension().string();
	ModelLoader loader;
	try {
		loader = ModelLoader(make_scope<AssimpModelLoader>(props.initial_rotation), props.path, props.flags);
	} catch (const CouldNotLoadModelException& exc) {
		Log::error("Mesh", "Model could not be loaded: {}", exc.what());
		return;
	}
	load_and_initialise_model(loader.get_mesh_data());
}

auto Mesh::get_indices() const -> const Disarray::IndexBuffer& { return *index_buffer; }

auto Mesh::get_vertices() const -> const Disarray::VertexBuffer& { return *vertex_buffer; }

void Mesh::force_recreation() { load_and_initialise_model(); }

auto Mesh::get_textures() const -> const Collections::RefVector<Disarray::Texture>& { return mesh_textures; }

auto Mesh::get_submeshes() const -> const Collections::ScopedStringMap<Disarray::Mesh>& { return submeshes; }

auto Mesh::has_children() const -> bool { return submeshes.size() > 0ULL; }

struct MeshConstructor {
	auto operator()(const auto& device, MeshProperties props) const { return Mesh::construct(device, std::move(props)); }
};

auto Mesh::construct_deferred(const Disarray::Device& device, MeshProperties properties) -> std::future<Ref<Disarray::Mesh>>
{
	return std::async(std::launch::async, MeshConstructor {}, std::cref(device), std::move(properties));
}

auto Mesh::get_aabb() const -> const AABB& { return aabb; }

auto Mesh::invalid() const -> bool
{
#ifdef IS_RELEASE
	return false;
#else
	return get_vertices().size() == 0 || get_indices().size() == 0;
#endif
}

} // namespace Disarray::Vulkan
