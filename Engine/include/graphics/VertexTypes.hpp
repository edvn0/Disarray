#pragma once

#include "core/Concepts.hpp"

#include <glm/glm.hpp>

namespace Disarray {

template <class T> inline constexpr auto vertex_per_object_count = 0;
template <class T> inline constexpr auto index_per_object_count = 0;

struct QuadVertex {
	glm::vec3 pos { 1.0f };
	glm::vec2 uvs { 1.0f };
	glm::vec3 normals { 1.0f };
	glm::vec4 colour { 1.0f };
	std::uint32_t identifier { 0 };
};
template <> inline constexpr auto vertex_per_object_count<QuadVertex> = 4;
template <> inline constexpr auto index_per_object_count<QuadVertex> = 6;

struct LineVertex {
	glm::vec3 pos { 1.0f };
	glm::vec4 colour { 1.0f };
};
template <> inline constexpr auto vertex_per_object_count<LineVertex> = 2;
template <> inline constexpr auto index_per_object_count<LineVertex> = 2;

struct LineIdVertex {
	glm::vec3 pos { 1.0f };
	glm::vec4 colour { 1.0f };
	std::uint32_t identifier { 0 };
};
template <> inline constexpr auto vertex_per_object_count<LineIdVertex> = 2;
template <> inline constexpr auto index_per_object_count<LineIdVertex> = 2;

template <class T>
concept IsValidVertexType = AnyOf<T, QuadVertex, LineVertex, LineIdVertex> && vertex_per_object_count<T> != 0 && index_per_object_count<T> != 0;

} // namespace Disarray
