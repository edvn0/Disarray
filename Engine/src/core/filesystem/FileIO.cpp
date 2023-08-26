#include "DisarrayPCH.hpp"

#include "core/filesystem/FileIO.hpp"

#include <filesystem>
#include <fstream>
#include <ostream>

#include "core/Formatters.hpp"
#include "core/Log.hpp"

namespace Disarray::FS {

void write_to_file(std::string_view path_sv, std::size_t size, const void* data)
{
	std::filesystem::path path { path_sv };
	std::ofstream stream { path };
	if (!stream) {
		Log::empty_error("Could not open file: {}", path);
		return;
	}

	stream.write(Disarray::bit_cast<const char*>(data), size);
}

} // namespace Disarray::FS
