#include "DisarrayPCH.hpp"

#include "core/Formatters.hpp"
#include "core/filesystem/FileIO.hpp"

#include <filesystem>
#include <fstream>
#include <ostream>

namespace Disarray::FS {

void write_to_file(std::string_view path_sv, std::size_t size, const void* data)
{
	std::filesystem::path path { path_sv };
	std::ofstream stream { path };
	if (!stream) {
		Log::empty_error("Could not write data to file at {}", path);
		return;
	}

	stream.write(Disarray::bit_cast<const char*>(data), size);
}

} // namespace Disarray::FS
