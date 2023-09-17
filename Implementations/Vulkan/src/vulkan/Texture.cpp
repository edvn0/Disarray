#include "DisarrayPCH.hpp"

#include "vulkan/Texture.hpp"

#include <utility>

#include "core/DataBuffer.hpp"
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
	auto pixels = load_pixels();

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
			.should_initialise_directly = props.should_initialise_directly,
			.debug_name = props.debug_name,
		});
}

Texture::Texture(const CommandExecutor* command_executor, const Disarray::Device& dev, Disarray::TextureProperties properties)
	: Disarray::Texture(std::move(properties))
	, device(dev)
{
	auto pixels = load_pixels();

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
			.should_initialise_directly = props.should_initialise_directly,
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

} // namespace Disarray::Vulkan
