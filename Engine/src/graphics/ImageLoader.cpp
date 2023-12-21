#include "DisarrayPCH.hpp"

#include <stb_image.h>

#include <cstdint>
#include <filesystem>
#include <stdexcept>

#include "core/DataBuffer.hpp"
#include "core/Ensure.hpp"
#include "core/Formatters.hpp"
#include "core/Log.hpp"
#include "core/filesystem/FileIO.hpp"
#include "graphics/ImageLoader.hpp"

namespace Disarray {

ImageLoader::ImageLoader(const std::filesystem::path& path, Disarray::DataBuffer& buffer)
{
	int tex_width, tex_height;

	if (!FS::exists(path)) {
		data = nullptr;
		Log::error("ImageLoader", "Texture {} did not exist.", path);
		return;
	}

	int requested = STBI_rgb_alpha;

	data = stbi_load(path.string().c_str(), &tex_width, &tex_height, &channels, requested);
	if (data == nullptr) {
		Log::error("ImageLoader", "Failed to load texture {}. Reason: {}", path, stbi_failure_reason());
		return;
	}

	size = tex_width * tex_height * requested;

	extent.width = static_cast<std::uint32_t>(tex_width);
	extent.height = static_cast<std::uint32_t>(tex_height);

	const DataBuffer data_buffer { data, size };
	buffer.copy_from(data_buffer);
}

ImageLoader::~ImageLoader() { free(); }

void ImageLoader::free()
{
	if (!data)
		return;
	stbi_image_free(data);
	data = nullptr;
}

} // namespace Disarray
