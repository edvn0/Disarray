#include "DisarrayPCH.hpp"

#include "graphics/Texture.hpp"

#include "vulkan/Texture.hpp"

namespace Disarray {

auto Texture::construct(const Disarray::Device& device, Disarray::TextureProperties properties) -> Ref<Disarray::Texture>
{
	return make_ref<Vulkan::Texture>(device, std::move(properties));
}

} // namespace Disarray
