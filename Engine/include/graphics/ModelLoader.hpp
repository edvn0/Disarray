#pragma once

#include <string>
#include <vector>

#include "graphics/ModelVertex.hpp"

namespace Disarray {

class ModelLoader {
public:
	explicit ModelLoader(const std::filesystem::path&, const glm::mat4& = glm::mat4 { 1.0f });
	~ModelLoader() = default;

	const auto& get_vertices() const { return vertices; }
	const auto& get_indices() const { return indices; }

	const auto get_vertices_size() const { return vertices.size() * sizeof(ModelVertex); }
	const auto get_indices_size() const { return indices.size() * sizeof(std::uint32_t); }

	const auto get_vertices_count() const { return vertices.size(); }
	const auto get_indices_count() const { return indices.size(); }

private:
	std::vector<ModelVertex> vertices;
	std::vector<uint32_t> indices;
};
} // namespace Disarray
