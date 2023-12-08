#include "DisarrayPCH.hpp"

#include <filesystem>
#include <fstream>
#include <ostream>

#include "core/Formatters.hpp"
#include "core/Log.hpp"
#include "core/filesystem/FileIO.hpp"

namespace Disarray::FS {

auto exists(const std::filesystem::path& path) -> bool { return std::filesystem::exists(path); }

} // namespace Disarray::FS
