#pragma once

#include "core/DataBuffer.hpp"
#include "graphics/Texture.hpp"
#include "vulkan/Image.hpp"
#include "vulkan/PropertySupplier.hpp"

namespace Disarray::Vulkan {

	class Texture : public Disarray::Texture {
	public:
		Texture(Device&, const TextureProperties&);
		~Texture() override;

		void force_recreation() override { recreate_texture(); };
		void recreate(bool should_clean, const Extent& extent) override
		{
			props.extent = extent;
			recreate_texture(should_clean);
		}
		VkImageView get_view() { return image->get_descriptor_info().imageView; }

		Image& get_image() override { return *image; }
		const Image& get_image() const override { return *image; }

		const TextureProperties& get_properties() const override { return props; }
		TextureProperties& get_properties() override { return props; }

	private:
		void recreate_texture(bool should_clean = true);
		void load_pixels();

		DataBuffer pixels;

		Device& device;

		Scope<Vulkan::Image> image;
		TextureProperties props;
	};

} // namespace Disarray::Vulkan
