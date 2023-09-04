#pragma once

#include "core/DataBuffer.hpp"
#include "core/Formatters.hpp"
#include "graphics/Texture.hpp"
#include "vulkan/Image.hpp"
#include "vulkan/PropertySupplier.hpp"

namespace Disarray::Vulkan {

class Texture : public Disarray::Texture {
	DISARRAY_MAKE_NONCOPYABLE(Texture)
public:
	Texture(const Disarray::Device&, TextureProperties);
	~Texture() override;

	void force_recreation() override { recreate_texture(); };
	void recreate(bool should_clean, const Extent& extent) override
	{
		auto& new_props = this->get_properties();
		if (!props.locked_extent) {
			new_props.extent = extent;
		}
		recreate_texture(should_clean);
	}
	auto get_view() -> VkImageView { return image->get_descriptor_info().imageView; }

	auto get_image() -> Disarray::Image& override { return *image; }
	auto get_image() const -> const Disarray::Image& override { return *image; }

private:
	void recreate_texture(bool should_clean = true);
	void load_pixels();

	DataBuffer pixels;

	const Disarray::Device& device;

	Scope<Vulkan::Image> image;
	std::filesystem::path cached_path;
};

} // namespace Disarray::Vulkan
