#pragma once

#include <glm/glm.hpp>

#include "core/Concepts.hpp"

namespace Disarray {

template <class T> inline constexpr std::size_t vertex_per_object_count = 0;
template <class T> inline constexpr std::size_t index_per_object_count = 0;

struct QuadVertex {
	glm::vec3 pos { 1.0F };
	glm::vec2 uvs { 1.0F };
	glm::vec3 normals { 1.0F };
	glm::vec4 colour { 1.0F };
};
template <> inline constexpr std::size_t vertex_per_object_count<QuadVertex> = 4;
template <> inline constexpr std::size_t index_per_object_count<QuadVertex> = 6;

struct LineVertex {
	glm::vec3 pos { 1.0F };
	glm::vec4 colour { 1.0F };
};
template <> inline constexpr std::size_t vertex_per_object_count<LineVertex> = 2;
template <> inline constexpr std::size_t index_per_object_count<LineVertex> = 2;

template <class T>
concept IsValidVertexType = AnyOf<T, QuadVertex, LineVertex> && vertex_per_object_count<T> != 0 && index_per_object_count<T> != 0;

} // namespace Disarray
