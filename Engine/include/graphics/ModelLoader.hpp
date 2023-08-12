#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace Disarray {

	struct Vertex {
		glm::vec3 pos;
		glm::vec2 uvs;
		glm::vec4 color;
		glm::vec3 normals;

		bool operator==(const Vertex& other) const
		{
			return pos == other.pos && color == other.color && uvs == other.uvs && normals == other.normals;
		}

		void rotate_by(glm::mat4 rotation)
		{
			pos = glm::vec4(pos, 1.0f) * rotation;
			normals = glm::vec4(normals, 1.0f) * rotation;
		}
	};

	class ModelLoader {
	public:
		explicit ModelLoader(const std::filesystem::path&, const glm::mat4& = glm::mat4 { 1.0f });
		~ModelLoader() = default;

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
} // namespace Disarray
