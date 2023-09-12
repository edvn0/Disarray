#include "DisarrayPCH.hpp"

#include "graphics/ModelLoader.hpp"

#include <glm/ext/matrix_transform.hpp>

#include <tinyobjloader.h>

#include <algorithm>
#include <unordered_map>

#include "core/Collections.hpp"
#include "core/exceptions/GeneralExceptions.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

namespace Disarray {

ModelLoader::ModelLoader(const std::filesystem::path& path, const glm::mat4& initial_rotation)
{
	const bool needs_rotate = initial_rotation != glm::mat4 { 1.0f };
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn;

	if (std::string err; !tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str())) {
		if (warn.empty()) {
			throw CouldNotLoadModelException(fmt::format("Error: {}", err));
		}

		throw CouldNotLoadModelException(fmt::format("Error: {}, Warning: {}", err, warn));
	}

	std::unordered_map<ModelVertex, uint32_t> unique_vertices {};

	std::vector<std::vector<ModelVertex>> tasks {};

	std::mutex attrib_mutex;
	for (const auto& shape : shapes) {
		auto split = Collections::split_into_batches<tinyobj::index_t, 4>(shape.mesh.indices);
		for (auto& sub_split : split) {
			tasks.emplace_back(Collections::map(sub_split, [&attrib, &mutex = attrib_mutex](const auto& index) {
				std::scoped_lock lock { mutex };
				ModelVertex vertex {};

				vertex.pos = { attrib.vertices[3 * index.vertex_index + 0], attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2] };

				vertex.uvs = { attrib.texcoords[2 * index.texcoord_index + 0], 1.0f - attrib.texcoords[2 * index.texcoord_index + 1] };

				vertex.color = { 1.0f, 1.0f, 1.0f, 1.0f };

				vertex.normals = { attrib.normals[3 * index.normal_index + 0], attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2] };
				return vertex;
			}));
		};
	}

	for (auto& vec : tasks) {
		for (const auto& vertex : vec) {
			if (!unique_vertices.contains(vertex)) {
				unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(unique_vertices[vertex]);
		}
	}

	if (needs_rotate) {
		Collections::parallel_for_each(vertices, [&rot = initial_rotation](auto& vertex) { vertex.rotate_by(rot); });
	}

	if (get_vertices_count() == 0 || get_indices_count() == 0) {
		throw CouldNotLoadModelException("Model was empty.");
	}
}

auto ModelLoader::get_vertices_count() const -> std::size_t { return vertices.size(); }

auto ModelLoader::get_indices_size() const -> std::size_t { return indices.size() * sizeof(std::uint32_t); }

auto ModelLoader::get_vertices_size() const -> std::size_t { return vertices.size() * sizeof(ModelVertex); }

auto ModelLoader::get_indices_count() const -> std::size_t { return indices.size(); }

} // namespace Disarray
