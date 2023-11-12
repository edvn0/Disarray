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
	void submit_billboarded_text(std::string_view text, const glm::mat4& transform, float scale = 1.0F, const glm::vec4& colour = { 1, 1, 1, 1 });
	void render(Disarray::Renderer& renderer, Disarray::CommandExecutor& executor);
	void clear_pass(Disarray::Renderer& renderer, Disarray::CommandExecutor& executor);
	auto recreate(bool should_clean, const Extent& extent) -> void;

	auto get_pipelines() -> std::array<Disarray::Pipeline*, 2>;

private:
	void draw_screen_space(Disarray::Renderer& renderer, Disarray::CommandExecutor& executor);
	void draw_world_space(Disarray::Renderer& renderer, Disarray::CommandExecutor& executor);
	void draw_billboard_space(Disarray::Renderer& renderer, Disarray::CommandExecutor& executor);

	struct TextRenderingAPI;
	Scope<TextRenderingAPI, PimplDeleter<TextRenderingAPI>> renderer_api { nullptr };

	struct FontData {
		Scope<Disarray::Texture> texture;
		glm::ivec2 size;
		glm::ivec2 bearing;
		std::uint32_t advance;
	};
	std::array<FontData, font_data_count> font_data;

	template <class T, std::size_t Count> struct GeneralTextData {
		std::array<T, static_cast<std::size_t>(4) * Count> text_data_buffer {};
		std::array<std::uint32_t, Count> character_texture_data {};
		std::array<glm::vec4, Count> character_colour_data {};
		std::uint32_t text_data_index { 0 };
		std::uint32_t vertex_data_index { 0 };
		std::uint32_t submitted_vertices { 0 };

		void reset()
		{
			text_data_index = 0;
			vertex_data_index = 0;
			submitted_vertices = 0;
			text_data_buffer.fill({});
			character_texture_data.fill({});
			character_colour_data.fill({});
		}

		auto submit_size() -> std::size_t { return sizeof(T) * vertex_data_index; }
	};

	struct ScreenSpaceTextData {
		glm::vec2 pos;
		glm::vec2 tex_coords;
	};
	GeneralTextData<ScreenSpaceTextData, glyph_count> screen_space;

	struct WorldSpaceTextData {
		glm::vec3 pos;
		glm::vec2 tex_coords;
	};
	GeneralTextData<WorldSpaceTextData, glyph_count> world_space;

	GeneralTextData<WorldSpaceTextData, glyph_count> billboard_space;

	std::uint32_t max_height { 0 };
};

} // namespace Disarray
