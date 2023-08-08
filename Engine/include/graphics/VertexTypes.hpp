#pragma once

#include "core/Concepts.hpp"

#include <glm/glm.hpp>

namespace Disarray {

	template <class T> inline constexpr auto vertex_count = 0;

	struct QuadVertex {
		glm::vec3 pos { 1.0f };
		glm::vec2 uvs { 1.0f };
		glm::vec2 normals { 1.0f };
		glm::vec4 colour { 1.0f };
		std::uint32_t identifier { 0 };
	};
	template <> inline constexpr auto vertex_count<QuadVertex> = 4;

	struct LineVertex {
		glm::vec3 pos { 1.0f };
		glm::vec4 colour { 1.0f };
	};
	template <> inline constexpr auto vertex_count<LineVertex> = 4;

	template <class T>
	concept IsValidVertexType = AnyOf<T, QuadVertex, LineVertex> && vertex_count<T> != 0;

} // namespace Disarray
