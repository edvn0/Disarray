#pragma once

#include "Forward.hpp"

#include <glm/glm.hpp>

#include <array>

#include "graphics/CommandExecutor.hpp"
#include "graphics/Pipeline.hpp"

namespace Disarray {

class TextRenderer {
	static constexpr auto glyph_count = 500UL;
	static constexpr auto font_data_count = 128UL;

public:
	TextRenderer() = default;

	void construct(Disarray::Renderer& renderer, const Disarray::Device& device, const Extent& extent);

	void submit_text(std::string_view text, const glm::uvec2& position, float scale = 1.0F, const glm::vec4& colour = { 1, 1, 1, 1 });
	void submit_text(std::string_view text, const glm::vec3& position, float scale = 1.0F, const glm::vec4& colour = { 1, 1, 1, 1 });
	void submit_text(std::string_view text, const glm::mat4& transform, float scale = 1.0F, const glm::vec4& colour = { 1, 1, 1, 1 });
	void render(Disarray::Renderer& renderer, Disarray::CommandExecutor& executor);
	void clear_pass(Disarray::Renderer& renderer, Disarray::CommandExecutor& executor);
	auto recreate(bool should_clean, const Extent& extent) -> void;

	auto get_pipelines() -> std::array<Disarray::Pipeline*, 2>;

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

	struct ScreenSpaceTextData {
		glm::vec2 pos;
		glm::vec2 tex_coords;
	};
	std::array<ScreenSpaceTextData, 4 * glyph_count> screen_space_text_data {};
	std::array<std::uint32_t, glyph_count> screen_space_text_character_texture_data {};
	std::array<glm::vec4, glyph_count> screen_space_text_character_colour_data {};
	struct WorldSpaceTextData {
		glm::vec3 pos;
		glm::vec2 tex_coords;
	};
	std::array<WorldSpaceTextData, 4 * glyph_count> world_space_text_data {};
	std::array<std::uint32_t, glyph_count> world_space_text_character_texture_data {};
	std::array<glm::vec4, glyph_count> world_space_text_character_colour_data {};

	std::uint32_t screen_space_text_data_index { 0 };
	std::uint32_t screen_space_vertex_data_index { 0 };
	std::uint32_t screen_space_submitted_vertices { 0 };

	std::uint32_t world_space_text_data_index { 0 };
	std::uint32_t world_space_vertex_data_index { 0 };
	std::uint32_t world_space_submitted_vertices { 0 };

	std::uint32_t max_height { 0 };
};

} // namespace Disarray
