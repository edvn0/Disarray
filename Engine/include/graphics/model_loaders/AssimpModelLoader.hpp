#pragma once

#include <glm/glm.hpp>

#include <filesystem>

#include "graphics/ModelLoader.hpp"

namespace Disarray {

struct AssimpModelLoader final : public IModelImporter {
	AssimpModelLoader() = default;
	auto import(const std::filesystem::path& path) -> ImportedMesh final;
};

} // namespace Disarray
