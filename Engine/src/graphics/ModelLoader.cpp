#include "graphics/ModelLoader.hpp"
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace std {
	template<> struct hash<Disarray::Vertex> {
		size_t operator()(Disarray::Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
						(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.uvs) << 1);
		}
	};
}

namespace Disarray {

	ModelLoader::ModelLoader(const std::string& path)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
			throw std::runtime_error(warn + err);
		}

		std::unordered_map<Vertex, uint32_t> unique_vertices{};

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex{};

				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.uvs = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				vertex.color = {1.0f, 1.0f, 1.0f, 1.0f};

				if (unique_vertices.count(vertex) == 0) {
					unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(unique_vertices[vertex]);
			}
		}
	}

}