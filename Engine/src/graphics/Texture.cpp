#include "DisarrayPCH.hpp"

#include "graphics/Texture.hpp"

#include "vulkan/Texture.hpp"

namespace Disarray {

auto Texture::construct(const Disarray::Device& device, Disarray::TextureProperties properties) -> Ref<Disarray::Texture>
{
	return make_ref<Vulkan::Texture>(device, std::move(properties));
}

auto Texture::construct_scoped(const Disarray::Device& device, Disarray::TextureProperties properties) -> Scope<Disarray::Texture>
{
	return make_scope<Vulkan::Texture>(device, std::move(properties));
}

} // namespace Disarray
