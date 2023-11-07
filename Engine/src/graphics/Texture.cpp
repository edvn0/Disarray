#include "DisarrayPCH.hpp"

#include "graphics/Texture.hpp"

#include "vulkan/Texture.hpp"

namespace Disarray {

auto Texture::construct(const Disarray::Device& device, Disarray::TextureProperties properties) -> Ref<Disarray::Texture>
{
	if (properties.dimension == TextureDimension::Two) {
		return make_ref<Vulkan::Texture>(device, std::move(properties));
	}
	return make_ref<Vulkan::Texture3D>(device, std::move(properties));
}

auto Texture::construct_scoped(const Disarray::Device& device, Disarray::TextureProperties properties) -> Scope<Disarray::Texture>
{
	if (properties.dimension == TextureDimension::Two) {
		return make_scope<Vulkan::Texture>(device, std::move(properties));
	}
	return make_scope<Vulkan::Texture3D>(device, std::move(properties));
}

void Texture::generate_mips(float count_images)
{
	if (props.generate_mips) {
		const auto float_extent = props.extent.as<float>();
		const auto maxed = std::max(float_extent.width, float_extent.height) / count_images;
		const auto logged = std::log2(maxed);
		const auto floored = std::floor(logged);
		props.mips = static_cast<std::uint32_t>(floored) + 1;
	}
}

} // namespace Disarray
