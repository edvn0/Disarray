#pragma once

#include <glm/glm.hpp>

#include <array>

#include "Forward.hpp"

namespace Disarray {

class TextRenderer {
	static constexpr auto glyph_count = 30UL;
	static constexpr auto font_data_count = 128UL;

public:
	TextRenderer() = default;

	void construct(Disarray::Renderer& renderer, const Disarray::Device& device, const Extent& extent);

	void submit_text(std::string_view text, const glm::uvec2& position, float scale = 1.0F);
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
	std::array<FontData, font_data_count> font_data;

	struct TextData {
		glm::vec2 pos;
		glm::vec2 tex_coords;
	};
	std::array<TextData, 4 * glyph_count> text_data {};
	std::array<std::uint32_t, glyph_count> text_character_texture_data {};

	std::uint32_t text_data_index { 0 };
	std::uint32_t vertex_data_index { 0 };
	std::uint32_t submitted_vertices { 0 };

	std::uint32_t max_height { 0 };
};

} // namespace Disarray
