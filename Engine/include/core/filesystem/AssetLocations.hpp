#pragma once

#include <filesystem>

#include "core/Concepts.hpp"

namespace Disarray::FS {

void setup_default_asset_path(std::filesystem::path asset_path);

auto asset_path() -> std::filesystem::path;

inline auto texture_directory() { return asset_path() / std::filesystem::path { "Textures" }; }
inline auto texture(Pathlike auto path) -> std::filesystem::path { return texture_directory() / std::filesystem::path { path }; }

inline auto shader_directory() { return asset_path() / std::filesystem::path { "Shaders" }; }
inline auto shader(Pathlike auto path) -> std::filesystem::path { return shader_directory() / std::filesystem::path { path }; }

inline auto model_directory() { return asset_path() / std::filesystem::path { "Models" }; }
inline auto model(Pathlike auto path) -> std::filesystem::path { return model_directory() / std::filesystem::path { path }; }

inline auto icon_directory() { return asset_path() / std::filesystem::path { "Icons" }; }
inline auto icon(Pathlike auto path) -> std::filesystem::path { return icon_directory() / std::filesystem::path { path }; }

} // namespace Disarray::FS