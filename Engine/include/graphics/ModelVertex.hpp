#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

namespace Disarray {

struct ModelVertex {
	glm::vec3 pos {};
	glm::vec2 uvs {};
	glm::vec4 color {};
	glm::vec3 normals {};
	glm::vec3 tangents {};
	glm::vec3 bitangents {};

	auto operator==(const ModelVertex& other) const -> bool
	{
		return pos == other.pos && color == other.color && uvs == other.uvs && normals == other.normals;
	}

	template <class T> void rotate_by(T&& rotation)
	{
		pos = glm::vec4(pos, 1.0F) * std::forward<T>(rotation);
		normals = glm::vec4(normals, 1.0F) * std::forward<T>(rotation);
	}
};

} // namespace Disarray

namespace std {
template <> struct hash<Disarray::ModelVertex> {
	auto operator()(const Disarray::ModelVertex& vertex) const noexcept -> std::size_t
	{
		const auto model_vertex_hash = hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1) ^ (hash<glm::vec2>()(vertex.uvs) << 1)
			^ ((hash<glm::vec3>()(vertex.normals) << 1) >> 1) ^ ((hash<glm::vec3>()(vertex.tangents) << 1) >> 1)
			^ ((hash<glm::vec3>()(vertex.bitangents) << 1) >> 1);

		return model_vertex_hash;
	}
};
} // namespace std
