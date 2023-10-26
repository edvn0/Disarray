#include "DisarrayPCH.hpp"

#include "vulkan/Texture.hpp"

#include <ktx.h>
#include <ktxvulkan.h>

#include <utility>

#include "core/DataBuffer.hpp"
#include "core/Ensure.hpp"
#include "core/Log.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Image.hpp"
#include "graphics/ImageLoader.hpp"
#include "vulkan/Image.hpp"

namespace Disarray::Vulkan {

Texture::Texture(const Disarray::Device& dev, Disarray::TextureProperties properties)
	: Disarray::Texture(std::move(properties))
	, device(dev)
{
	DataBuffer pixels = props.data_buffer.is_valid() ? props.data_buffer : load_pixels();

	if (props.generate_mips) {
		props.mips = static_cast<std::uint32_t>(std::floor(std::log2(std::max(props.extent.width, props.extent.height)))) + 1;
	}
	image = make_scope<Vulkan::Image>(device,
		ImageProperties {
			.extent = props.extent,
			.format = props.format,
			.data = std::move(pixels),
			.mips = props.generate_mips ? *props.mips : 1,
			.locked_extent = props.locked_extent,
			.sampler_modes = {
				.u = props.sampler_modes.u,
				.v = props.sampler_modes.v,
				.w = props.sampler_modes.w,
			},
			.border_colour = props.border_colour,
			.debug_name = props.debug_name,
		});
}

Texture::Texture(const CommandExecutor* command_executor, const Disarray::Device& dev, Disarray::TextureProperties properties)
	: Disarray::Texture(std::move(properties))
	, device(dev)
{
	DataBuffer pixels = props.data_buffer.is_valid() ? props.data_buffer : load_pixels();
	if (!props.mips) {
		props.mips = static_cast<std::uint32_t>(std::floor(std::log2(std::max(props.extent.width, props.extent.height)))) + 1;
	}
	image = make_scope<Vulkan::Image>(command_executor, device,
		ImageProperties {
			.extent = props.extent,
			.format = props.format,
			.data = std::move(pixels),
			.mips = *props.mips,
			.locked_extent = props.locked_extent,
			.debug_name = props.debug_name,
		});
}

Texture::~Texture() = default;

void Texture::recreate_texture(bool should_clean)
{
	auto pixels = load_pixels();
	image->get_properties().data.copy_from(pixels);
	image->recreate(should_clean, props.extent);
}

auto Texture::load_pixels() -> DataBuffer
{
	DataBuffer pixels {};
	if (!props.path.empty()) {
		ImageLoader loader { props.path, pixels };
		props.extent = loader.get_extent();
	}
	return pixels;
}

Texture3D::Texture3D(const Device& dev, TextureProperties properties)
	: Disarray::Texture(std::move(properties))
	, device(dev)
{

	Extent full_extent;
	DataBuffer pixels {};
	std::vector<CopyRegion> copy_regions;
	if (!props.path.empty()) {
		ktxResult result;
		ktxTexture* texture_data;

		result = ktxTexture_CreateFromNamedFile(props.path.string().c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &texture_data);
		ensure(result == KTX_SUCCESS, "Could not load image.");

		// Get properties required for using and upload texture data from the ktx texture object
		full_extent.width = texture_data->baseWidth;
		full_extent.height = texture_data->baseHeight;
		props.mips = texture_data->numLevels;
		ktx_uint8_t* ktxTextureData = ktxTexture_GetData(texture_data);
		const auto ktxTextureSize = ktxTexture_GetDataSizeUncompressed(texture_data);

		pixels.copy_from(DataBuffer { ktxTextureData, ktxTextureSize });

		// Setup buffer copy regions for each face including all of its miplevels
		for (uint32_t face = 0; face < 6; face++) {
			for (uint32_t level = 0; level < props.mips; level++) {
				// Calculate offset into staging buffer for the current mip level and face
				ktx_size_t offset;
				KTX_error_code ret = ktxTexture_GetImageOffset(texture_data, level, 0, face, &offset);
				assert(ret == KTX_SUCCESS);
				CopyRegion bufferCopyRegion = {};
				bufferCopyRegion.mip_level = level;
				bufferCopyRegion.base_array_layer = face;
				bufferCopyRegion.width = texture_data->baseWidth >> level;
				bufferCopyRegion.height = texture_data->baseHeight >> level;
				bufferCopyRegion.buffer_offset = static_cast<std::uint32_t>(offset);
				copy_regions.push_back(bufferCopyRegion);
			}
		}

		ktxTexture_Destroy(texture_data);
	}

	ImageProperties image_properties {
		.extent = full_extent,
		.format = props.format,
		.data = std::move(pixels),
		.mips = *props.mips,
		.copy_regions = copy_regions,
		.layers = 6,
		.dimension = ImageDimension::Three,
		.debug_name = props.debug_name,
	};
	image = make_scope<Vulkan::Image>(device, std::move(image_properties));
}

void Texture3D::recreate_texture(bool should_clean) { }

Texture3D::~Texture3D() = default;

} // namespace Disarray::Vulkan
