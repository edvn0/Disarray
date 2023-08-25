#pragma once

#include <vulkan/vulkan.h>

#include "PropertySupplier.hpp"
#include "core/UniquelyIdentifiable.hpp"
#include "graphics/Image.hpp"
#include "vulkan/MemoryAllocator.hpp"

namespace Disarray::Vulkan {

struct ImageInfo {
	VkImage image { nullptr };
	VkImageView view { nullptr };
	VkSampler sampler { nullptr };
	VmaAllocation allocation { nullptr };
};

static constexpr auto to_vulkan_format(ImageFormat format)
{
	switch (format) {
	case ImageFormat::SRGB:
		return VK_FORMAT_R8G8B8A8_SRGB;
	case ImageFormat::RGB:
		return VK_FORMAT_R8G8B8_SRGB;
	case ImageFormat::SBGR:
		return VK_FORMAT_B8G8R8A8_SRGB;
	case ImageFormat::BGR:
		return VK_FORMAT_B8G8R8_SRGB;
	case ImageFormat::SRGB32:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case ImageFormat::RGB32:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case ImageFormat::Uint:
		return VK_FORMAT_R32_UINT;
	case ImageFormat::Depth:
		return VK_FORMAT_D32_SFLOAT;
	case ImageFormat::DepthStencil:
		return VK_FORMAT_D32_SFLOAT_S8_UINT;
	default:
		unreachable();
	}
}

static constexpr auto is_depth_format = [](ImageFormat format) { return format == ImageFormat::Depth || format == ImageFormat::DepthStencil; };

constexpr VkImageLayout to_vulkan_layout(ImageFormat format)
{
	switch (format) {
	case ImageFormat::SRGB:
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	case ImageFormat::RGB:
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	case ImageFormat::SRGB32:
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	case ImageFormat::RGB32:
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	case ImageFormat::SBGR:
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	case ImageFormat::BGR:
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	case ImageFormat::Uint:
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	case ImageFormat::Depth:
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	case ImageFormat::DepthStencil:
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	default:
		unreachable();
	}
}

constexpr VkSampleCountFlagBits to_vulkan_samples(SampleCount format)
{
	switch (format) {
	case SampleCount::One:
		return VK_SAMPLE_COUNT_1_BIT;
	case SampleCount::Two:
		return VK_SAMPLE_COUNT_2_BIT;
	case SampleCount::Four:
		return VK_SAMPLE_COUNT_4_BIT;
	case SampleCount::Eight:
		return VK_SAMPLE_COUNT_8_BIT;
	case SampleCount::Sixteen:
		return VK_SAMPLE_COUNT_16_BIT;
	case SampleCount::ThirtyTwo:
		return VK_SAMPLE_COUNT_32_BIT;
	case SampleCount::SixtyFour:
		return VK_SAMPLE_COUNT_64_BIT;
	default:
		unreachable();
	}
}

constexpr auto to_vulkan_tiling(Tiling tiling)
{
	switch (tiling) {
	default:
		unreachable();
	case Tiling::Linear:
		return VK_IMAGE_TILING_LINEAR;
	case Tiling::DeviceOptimal:
		return VK_IMAGE_TILING_OPTIMAL;
	}
}

class Image : public Disarray::Image, public PropertySupplier<VkImage> {
public:
	Image(const Disarray::Device&, const ImageProperties&);
	~Image() override;

	void force_recreation() override { recreate(true, props.extent); };
	void recreate(bool should_clean, const Extent&) override;

	PixelReadData read_pixel(const glm::vec2&) const override;
	const ImageProperties& get_properties() const override { return props; }

	VkImage get_image() const { return info.image; }
	const VkDescriptorSetLayout& get_layout() const { return layout; }

	VkImage supply() const override { return get_image(); }

	const VkDescriptorImageInfo& get_descriptor_info() const { return descriptor_info; }

	Identifier hash() const override
	{
		return bit_cast<std::uint64_t>(descriptor_info.imageView) ^ bit_cast<std::uint64_t>(descriptor_info.sampler);
	};

private:
	void recreate_image(bool should_clean);
	void update_descriptor();
	void destroy_resources();
	void create_mips();

	ImageInfo info {};
	VkDescriptorImageInfo descriptor_info;
	VkDescriptorSetLayout layout;

	const Disarray::Device& device;
	ImageProperties props;
};

} // namespace Disarray::Vulkan