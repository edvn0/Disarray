#pragma once

#include "Forward.hpp"
#include "graphics/ImageProperties.hpp"

namespace Disarray {

	struct TextureProperties {
		Extent extent;
		ImageFormat format;
		std::string path {};
	};

	class Texture {
	public:
		virtual ~Texture() = default;

		virtual void force_recreation() = 0;

		static Ref<Texture> construct(Ref<Device>, Ref<Swapchain>, Ref<PhysicalDevice>, const TextureProperties&);
	};

}