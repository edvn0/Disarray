#pragma once

#include <glm/glm.hpp>

#include <filesystem>

#include "glm/ext/matrix_transform.hpp"
#include "graphics/ModelLoader.hpp"

namespace Disarray {

struct AssimpModelLoader final : public IModelImporter {
	AssimpModelLoader(const glm::mat4& rot = glm::identity<glm::mat4>())
		: initial_rotation(rot) {

		};
	auto import(const std::filesystem::path& path) -> ImportedMesh final;

private:
	glm::mat4 initial_rotation { 1.0F };
};

} // namespace Disarray
