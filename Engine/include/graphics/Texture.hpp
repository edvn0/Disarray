#pragma once

#include <array>
#include <filesystem>
#include <optional>
#include <string>

#include "Forward.hpp"
#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Image.hpp"
#include "graphics/ImageProperties.hpp"

namespace Disarray {
struct TextureProperties {
	Extent extent {};
	ImageFormat format { ImageFormat::SRGB }; // TODO: This is a crazy default, just to shut up clangd...
	bool generate_mips { false };
	std::optional<std::uint32_t> mips { std::nullopt };
	std::filesystem::path path {};
	struct SamplerModeUVW {
		SamplerMode u = SamplerMode::Repeat;
		SamplerMode v = SamplerMode::Repeat;
		SamplerMode w = SamplerMode::Repeat;
	};
	SamplerModeUVW sampler_modes {};
	BorderColour border_colour { BorderColour::FloatOpaqueWhite };
	bool locked_extent { false };

	/**
	 * @brief To bake large commands buffers, we can batch textures with this set to false!
	 */
	bool should_initialise_directly { true };

	std::string debug_name;
};

class Texture : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(Texture, TextureProperties)
public:
	virtual auto get_image() const -> const Image& = 0;

	virtual void construct_using(CommandExecutor&) = 0;
};

} // namespace Disarray
