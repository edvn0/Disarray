#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace Disarray {

	struct Vertex {
		glm::vec3 pos;
		glm::vec2 uvs;
		glm::vec4 color;
		glm::vec3 normals;

		bool operator==(const Vertex& other) const {
			return pos == other.pos && color == other.color && uvs == other.uvs && normals == other.normals;
		}
	};

	class ModelLoader {
	public:
		explicit ModelLoader(const std::string&);

		const auto& get_vertices() const { return vertices; }
		const auto& get_indices() const { return indices; }

		const auto get_vertices_size() const { return vertices.size() * sizeof(Vertex); }
		const auto get_indices_size() const { return indices.size() * sizeof(std::uint32_t); }

		const auto get_vertices_count() const { return vertices.size(); }
		const auto get_indices_count() const { return indices.size(); }

	private:
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
	};
}
