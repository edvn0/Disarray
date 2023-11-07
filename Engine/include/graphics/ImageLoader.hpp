#pragma once

#include <string>

#include "ImageProperties.hpp"
#include "core/DataBuffer.hpp"

namespace Disarray {

class ImageLoader {
public:
	explicit ImageLoader(const std::filesystem::path&, DataBuffer&);
	~ImageLoader();

	const auto& get_extent() const { return extent; }

	auto get_channels() const { return channels; }

private:
	void free();

	Extent extent;
	void* data;
	std::uint64_t size;
	int channels { 0 };
};

class TextureCubeLoader {
public:
	explicit TextureCubeLoader(const std::filesystem::path&, DataBuffer&);
	~TextureCubeLoader();

private:
	std::array<Scope<std::byte[]>, 6> temporary_storage;
};

} // namespace Disarray
