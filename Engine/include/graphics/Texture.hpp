#pragma once

#include "Forward.hpp"
#include "core/ReferenceCounted.hpp"
#include "graphics/Device.hpp"
#include "graphics/Image.hpp"
#include "graphics/ImageProperties.hpp"
#include "graphics/Swapchain.hpp"

namespace Disarray {

	struct TextureProperties {
		Extent extent;
		ImageFormat format;
		std::string path {};
		bool should_present { false };
		std::string debug_name;
	};

	class Texture : public ReferenceCountable {
		DISARRAY_MAKE_REFERENCE_COUNTABLE(Texture)
	public:
		virtual void force_recreation() = 0;
		virtual void recreate(bool should_clean) = 0;

		virtual Image& get_image() = 0;

		static Ref<Texture> construct(Device&, Swapchain&, const TextureProperties&);
	};

} // namespace Disarray
