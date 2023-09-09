#pragma once

#include <string>
#include <vector>

#include "graphics/ModelVertex.hpp"

namespace Disarray {

class ModelLoader {
public:
	explicit ModelLoader(const std::filesystem::path&, const glm::mat4& = glm::mat4 { 1.0F });

	[[nodiscard]] auto get_vertices() const -> const auto& { return vertices; }
	[[nodiscard]] auto get_indices() const -> const auto& { return indices; }

	[[nodiscard]] auto get_vertices_size() const -> std::size_t;
	[[nodiscard]] auto get_indices_size() const -> std::size_t;

	[[nodiscard]] auto get_vertices_count() const -> std::size_t;
	[[nodiscard]] auto get_indices_count() const -> std::size_t;

private:
	std::vector<ModelVertex> vertices;
	std::vector<uint32_t> indices;
};
} // namespace Disarray
