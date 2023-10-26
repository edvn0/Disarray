#pragma once

#include "core/DataBuffer.hpp"
#include "core/Formatters.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Texture.hpp"
#include "vulkan/Image.hpp"
#include "vulkan/PropertySupplier.hpp"

namespace Disarray::Vulkan {

class Texture : public Disarray::Texture {
	DISARRAY_MAKE_NONCOPYABLE(Texture)
public:
	Texture(const Disarray::Device&, TextureProperties);
	Texture(const Disarray::CommandExecutor*, const Disarray::Device&, TextureProperties);
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

	auto get_image(std::uint32_t) const -> const Disarray::Image& override { return *image; }

	void construct_using(Disarray::CommandExecutor&) override {};

private:
	void recreate_texture(bool should_clean = true);
	auto load_pixels() -> DataBuffer;

	const Disarray::Device& device;

	Scope<Vulkan::Image> image;
	std::filesystem::path cached_path;
};

class Texture3D : public Disarray::Texture {
	DISARRAY_MAKE_NONCOPYABLE(Texture3D)
public:
	Texture3D(const Disarray::Device&, TextureProperties);
	Texture3D(const Disarray::CommandExecutor*, const Disarray::Device&, TextureProperties);
	~Texture3D() override;

	void force_recreation() override { recreate_texture(); };
	void recreate(bool should_clean, const Extent& extent) override
	{
		auto& new_props = this->get_properties();
		if (!props.locked_extent) {
			new_props.extent = extent;
		}
		recreate_texture(should_clean);
	}

	auto get_image(std::uint32_t index) const -> const Disarray::Image& override { return *image; }

	void construct_using(Disarray::CommandExecutor&) override {};

private:
	void recreate_texture(bool should_clean = true);
	auto load_pixels() -> DataBuffer;

	const Disarray::Device& device;

	Scope<Vulkan::Image> image;
	std::filesystem::path cached_path;
};

} // namespace Disarray::Vulkan
