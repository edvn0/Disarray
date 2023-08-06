#include "DisarrayPCH.hpp"

#include "graphics/ModelLoader.hpp"

#include "tinyobjloader.h"

#include <algorithm>
#include <glm/ext/matrix_transform.hpp>
#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <execution>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

namespace std {
	template <> struct hash<Disarray::Vertex> {
		size_t operator()(Disarray::Vertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.uvs) << 1)
				^ ((hash<glm::vec3>()(vertex.normals) << 1) >> 1);
		}
	};
} // namespace std

namespace Disarray {

	ModelLoader::ModelLoader(const std::string& path, const glm::mat4& initial_rotation)
	{
		const bool needs_rotate = initial_rotation != glm::mat4 { 1.0f };
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn;

		if (std::string err; !tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
			throw std::runtime_error(warn + err);
		}

		std::unordered_map<Vertex, uint32_t> unique_vertices {};

		for (const auto& shape : shapes) {
			std::for_each(std::begin(shape.mesh.indices), std::end(shape.mesh.indices), [&](const auto& index) {
				Vertex vertex {};

				vertex.pos = { attrib.vertices[3 * index.vertex_index + 0], attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2] };

				vertex.uvs = { attrib.texcoords[2 * index.texcoord_index + 0], 1.0f - attrib.texcoords[2 * index.texcoord_index + 1] };

				vertex.color = { 1.0f, 1.0f, 1.0f, 1.0f };

				vertex.normals = { attrib.normals[3 * index.normal_index + 0], attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2] };

				if (!unique_vertices.contains(vertex)) {
					unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(unique_vertices[vertex]);
			});
		}

		if (needs_rotate)
			std::for_each(
				std::execution::par, std::begin(vertices), std::end(vertices), [&rot = initial_rotation](auto& vertex) { vertex.rotate_by(rot); });
	}

} // namespace Disarray
