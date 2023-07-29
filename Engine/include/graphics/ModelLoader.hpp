#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace Disarray {

	struct Vertex {
		glm::vec3 pos;
		glm::vec2 uvs;
		glm::vec4 color;

		bool operator==(const Vertex& other) const {
			return pos == other.pos && color == other.color && uvs == other.uvs;
		}
	};

	class ModelLoader {
	public:
		ModelLoader(const std::string&);

		const auto& get_vertices() const { return vertices; }
		const auto& get_indices() const { return indices; }

		const auto get_vertices_size() const { return vertices.size() * sizeof(Vertex); }
		const auto get_indices_size() const { return indices.size() * sizeof(std::uint32_t); }

	private:
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
	};
}
