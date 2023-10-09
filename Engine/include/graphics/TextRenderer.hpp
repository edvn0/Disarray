#pragma once

#include <array>

#include "Forward.hpp"
#include "core/Concepts.hpp"

namespace Disarray {

class TextRenderer {
	static constexpr auto glyph_count = 1000UL;

public:
	TextRenderer() = default;

	void construct(Disarray::Renderer& renderer, const Disarray::Device& device, const Extent& extent);

	void submit_text(std::string_view text, const glm::uvec2&, float size = 1.0F);
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
	std::array<FontData, 128> font_data;

	struct TextData {
		std::size_t character_texture_index;
		unsigned char character;
		glm::ivec2 position;
		float glyph_width;
		float glyph_height;

		struct VertexData {
			glm::vec2 pos;
			glm::vec2 tex_coords;
		};
		std::array<VertexData, 4> vertices {};
	};
	std::array<TextData, glyph_count> text_data {};
	std::size_t text_data_index { 0 };
};

} // namespace Disarray
