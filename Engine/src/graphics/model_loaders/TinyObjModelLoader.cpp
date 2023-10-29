#include "graphics/model_loaders/TinyObjModelLoader.hpp"

#include <tinyobjloader.h>

#include "core/Log.hpp"
#include "graphics/ModelLoader.hpp"

namespace Disarray {

auto TinyObjModelLoader::import(const std::filesystem::path& path, ImportFlag) -> ImportedMesh
{
	std::vector<ModelVertex> vertices;
	std::vector<std::uint32_t> indices;

	const bool needs_rotate = initial_rotation != glm::mat4 { 1.0F };
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

	if (vertices.empty() || indices.empty()) {
		throw CouldNotLoadModelException("Model was empty.");
	}
	const auto& identifier = path.filename().replace_extension().string();

	Log::info("SimpleModelLoader", "Identifier: {}", identifier);
	return { { identifier, { vertices, indices } } };
}

} // namespace Disarray
