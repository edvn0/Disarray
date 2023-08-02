#include "DisarrayPCH.hpp"

#include "graphics/ImageLoader.hpp"

#include "core/DataBuffer.hpp"
#include "core/Log.hpp"

#include <cstdint>
#include <filesystem>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Disarray {

	ImageLoader::ImageLoader(const std::string& path, Disarray::DataBuffer& buffer)
	{
		int tex_width, tex_height, tex_channels;

		if (!std::filesystem::exists(path)) {
			Log::error("ImageLoader", "File does not exist.");
			data = nullptr;
			return;
		}

		data = stbi_load(path.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
		size = tex_width * tex_height * 4;

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
