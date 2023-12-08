#pragma once

#include "Forward.hpp"

#include <array>
#include <filesystem>
#include <optional>
#include <string>

#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Image.hpp"
#include "graphics/ImageProperties.hpp"

namespace Disarray {

enum class TextureDimension : std::uint8_t {
	Two,
	Three,
};

struct TextureProperties {
	Extent extent {};
	ImageFormat format { ImageFormat::SRGB }; // TODO(EdwinC): This is a crazy default, just to shut up clangd...
	bool generate_mips { false };
	std::optional<std::uint32_t> mips { std::nullopt };
	std::filesystem::path path {};
	DataBuffer data_buffer { nullptr };
	struct SamplerModeUVW {
		SamplerMode u = SamplerMode::Repeat;
		SamplerMode v = SamplerMode::Repeat;
		SamplerMode w = SamplerMode::Repeat;
	};
	SamplerModeUVW sampler_modes {};
	BorderColour border_colour { BorderColour::FloatOpaqueWhite };
	bool locked_extent { false };
	TextureDimension dimension { TextureDimension::Two };

	/**
	 * @brief To bake large commands buffers, we can batch textures with this set to false!
	 */
	bool should_initialise_directly { true };

	std::string debug_name;
};

class Texture : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(Texture, TextureProperties)
public:
	auto get_image() const -> const Image& { return get_image(0); };
	virtual auto get_image(std::uint32_t index) const -> const Image& = 0;

	virtual void construct_using(CommandExecutor&) = 0;
	virtual auto valid() const -> bool = 0;

	virtual auto hash() const -> std::size_t = 0;

protected:
	void generate_mips(float count_images = 0.0F);
};

} // namespace Disarray
