#pragma once

#include <filesystem>
#include <optional>
#include <string>

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
	std::filesystem::path path {};
	bool locked_extent { false };
	std::string debug_name;
};

class Texture : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(Texture, TextureProperties)
public:
	virtual auto get_image() -> Image& = 0;
	virtual auto get_image() const -> const Image& = 0;

	static auto construct(const Disarray::Device&, TextureProperties) -> Ref<Texture>;
};

} // namespace Disarray
