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

void Mesh::load_and_initialise_model()
{
	static constexpr auto find_index = [](const std::vector<Ref<Disarray::Texture>>& haystack, const Ref<Disarray::Texture>& needle) {
		std::int32_t index = 0;
		for (const auto& hay : haystack) {
			if (hay == needle) {
				return std::optional { index };
			}
			index++;
		}
		return std::optional<std::int32_t> { std::nullopt };
	};

	ModelLoader loader;
	try {
		loader = ModelLoader(make_scope<AssimpModelLoader>(props.initial_rotation), props.path, props.flags);
	} catch (const CouldNotLoadModelException& exc) {
		Log::error("Mesh", "Model could not be loaded: {}", exc.what());
		return;
	}

	mesh_textures = loader.construct_textures(device);
	aabb = loader.get_aabb();
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

		std::unordered_set<std::int32_t> image_indices {};
		for (const auto& tex : submesh.textures) {
			if (const auto found = find_index(mesh_textures, tex); found.has_value()) {
				image_indices.insert(*found);
			}
		}

		auto substructure = make_scope<MeshSubstructure>(std::move(vertex_buffer), std::move(index_buffer), std::move(image_indices));
		submeshes.try_emplace(key, std::move(substructure));
	}

	mesh_name = props.path.filename().replace_extension().string();
}

auto Mesh::get_indices() const -> Disarray::IndexBuffer&
{
	if (submeshes.contains(mesh_name)) {
		return *submeshes.at(mesh_name)->indices;
	}
	return *submeshes.begin()->second->indices;
}

auto Mesh::get_vertices() const -> Disarray::VertexBuffer&
{
	if (submeshes.contains(mesh_name)) {
		return *submeshes.at(mesh_name)->vertices;
	}
	return *submeshes.begin()->second->vertices;
}

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
