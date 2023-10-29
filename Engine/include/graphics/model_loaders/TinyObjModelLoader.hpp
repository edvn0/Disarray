#pragma once

#include <glm/glm.hpp>

#include <filesystem>

#include "graphics/ModelLoader.hpp"

namespace Disarray {

struct TinyObjModelLoader final : public IModelImporter {
	glm::mat4 initial_rotation;

	explicit TinyObjModelLoader(const glm::mat4& initial)
		: initial_rotation(initial)
	{
	}

	auto import(const std::filesystem::path& path, ImportFlag) -> ImportedMesh final;
};

} // namespace Disarray
