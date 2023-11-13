#include "DisarrayPCH.hpp"

#include <ktx.h>
#include <ktxvulkan.h>

#include <utility>

#include "../../../../ThirdParty/stb_image/include/stb_image_write.h"
#include "core/DataBuffer.hpp"
#include "core/Ensure.hpp"
#include "core/Log.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Image.hpp"
#include "graphics/ImageLoader.hpp"
#include "vulkan/Image.hpp"
#include "vulkan/Texture.hpp"

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

void extract_cubemap_faces(const std::filesystem::path& path)
{
	// Load the texture map image
	DataBuffer buffer;
	ImageLoader loader { path, buffer };
	auto&& [width, height] = loader.get_extent();
	auto channels = loader.get_channels();

	// Define the coordinates for the faces in the 4x3 grid
	std::uint32_t cellWidth = width / 4;
	std::uint32_t cellHeight = height / 3;

	if (width == height) {
		cellWidth = cellHeight = (cellWidth / 4);
	}

	struct FaceCoordinates {
		std::uint32_t x1, y1, x2, y2;
	};
	std::array<FaceCoordinates, 6> faces = {
		FaceCoordinates { cellWidth, 0, cellWidth * 2, cellHeight }, // top
		{ cellWidth * 0, cellHeight, cellWidth * 1, cellHeight * 2 }, // left
		{ cellWidth * 1, cellHeight, cellWidth * 2, cellHeight * 2 }, // front
		{ cellWidth * 2, cellHeight, cellWidth * 3, cellHeight * 2 }, // right
		{ cellWidth * 3, cellHeight, cellWidth * 4, cellHeight * 2 }, // back
		{ cellWidth, cellHeight * 2, cellWidth * 2, cellHeight * 3 }, // top
	};

	std::array<DataBuffer, 6> buffers {};
	for (auto i = 0ULL; i < faces.size(); ++i) {
		const FaceCoordinates& coordinates = faces[i];
		int faceWidth = coordinates.x2 - coordinates.x1;
		int faceHeight = coordinates.y2 - coordinates.y1;

		// Allocate memory for the face image
		auto& face_image = buffers.at(i);
		face_image.copy_from(DataBuffer(faceWidth * faceHeight * channels));

		// Copy pixels from the texture map to the face image
		for (int y = 0; y < faceHeight; ++y) {
			for (int x = 0; x < faceWidth; ++x) {
				int sourceX = coordinates.x1 + x;
				int sourceY = coordinates.y1 + y;
				int sourceIndex = (sourceY * width + sourceX) * channels;
				int destinationIndex = (y * faceWidth + x) * channels;

				for (int c = 0; c < channels; ++c) {
					face_image[destinationIndex + c] = buffer[sourceIndex + c];
				}
			}
		}
	}
}

Texture3D::Texture3D(const Device& dev, TextureProperties properties)
	: Disarray::Texture(std::move(properties))
	, device(dev)
{
	Extent full_extent;
	DataBuffer pixels {};
	std::vector<CopyRegion> copy_regions;
	if (!props.path.empty()) {
		// NOTE: We assume that PNGs are default texture cubemap layouted.
		ktxTexture* texture_data { nullptr };
		ktxResult result = KTX_FILE_DATA_ERROR;
		if (props.path.extension() == ".png") {
			extract_cubemap_faces(props.path);
			Log::error("Texture3D", "Cannot load PNGs just yet.");
			return;
		} else {
			result = ktxTexture_CreateFromNamedFile(props.path.string().c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &texture_data);
		}

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
		.sampler_modes = {
			.u = SamplerMode::ClampToEdge,
			.v = SamplerMode::ClampToEdge,
			.w = SamplerMode::ClampToEdge,
		},
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
