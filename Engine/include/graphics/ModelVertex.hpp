#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

namespace Disarray {

struct ModelVertex {
	glm::vec3 pos;
	glm::vec2 uvs;
	glm::vec4 color;
	glm::vec3 normals;

	bool operator==(const ModelVertex& other) const
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
	size_t operator()(Disarray::ModelVertex const& vertex) const
	{
		return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.uvs) << 1)
			^ ((hash<glm::vec3>()(vertex.normals) << 1) >> 1);
	}
};
} // namespace std
