#include "DisarrayPCH.hpp"

#include "graphics/Texture.hpp"

#include "vulkan/Texture.hpp"

namespace Disarray {

	Ref<Texture> Texture::construct(const Disarray::Device& device, const Disarray::TextureProperties& props)
	{
		return make_ref<Vulkan::Texture>(device, props);
	}

} // namespace Disarray
