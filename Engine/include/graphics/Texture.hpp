#pragma once

#include "Forward.hpp"
#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "graphics/Device.hpp"
#include "graphics/Image.hpp"
#include "graphics/ImageProperties.hpp"
#include "graphics/Swapchain.hpp"

namespace Disarray {

	struct TextureProperties {
		Extent extent;
		ImageFormat format;
		std::optional<std::uint32_t> mips { std::nullopt };
		std::string path {};
		std::string debug_name;
	};

	class Texture : public ReferenceCountable {
		DISARRAY_OBJECT(Texture)
	public:
		virtual Image& get_image() = 0;
		virtual const Image& get_image() const = 0;

		static Ref<Texture> construct(Device&, const TextureProperties&);
	};

} // namespace Disarray
