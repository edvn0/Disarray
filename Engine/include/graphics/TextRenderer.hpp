#pragma once

#include <array>

#include "Forward.hpp"
#include "core/Concepts.hpp"

namespace Disarray {

class TextRenderer {
	static constexpr auto glyph_count = 10000UL;

public:
	TextRenderer() = default;

	void construct(Disarray::Renderer& renderer, const Disarray::Device& device);

	void submit_text(std::string_view text, const glm::uvec2&, float size);
	void render(Disarray::Renderer& renderer, Disarray::CommandExecutor& executor);

private:
	struct TextRenderingAPI;
	Scope<TextRenderingAPI, PimplDeleter<TextRenderingAPI>> renderer_api { nullptr };

	struct FontData {
		Scope<Disarray::Texture> texture;
		glm::ivec2 size;
		glm::ivec2 bearing;
		std::uint32_t advance;
	};
	std::array<FontData, glyph_count> font_data;
};

} // namespace Disarray
