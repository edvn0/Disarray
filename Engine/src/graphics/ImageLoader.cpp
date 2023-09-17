#include "DisarrayPCH.hpp"

#include "graphics/ImageLoader.hpp"

#include <stb_image.h>

#include <filesystem>
#include <stdexcept>

#include "core/DataBuffer.hpp"
#include "core/Ensure.hpp"
#include "core/Formatters.hpp"
#include "core/Log.hpp"

namespace Disarray {

ImageLoader::ImageLoader(const std::filesystem::path& path, Disarray::DataBuffer& buffer)
{
	int tex_width, tex_height, tex_channels;

	if (!std::filesystem::exists(path)) {
		data = nullptr;
		Log::error("ImageLoader", "Texture {} did not exist.", path);
		return;
	}

	int requested = STBI_rgb_alpha;
	data = stbi_load(path.string().c_str(), &tex_width, &tex_height, &tex_channels, requested);
	size = tex_width * tex_height * requested;

	extent.width = static_cast<std::uint32_t>(tex_width);
	extent.height = static_cast<std::uint32_t>(tex_height);

	DataBuffer data_buffer { data, size };
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
