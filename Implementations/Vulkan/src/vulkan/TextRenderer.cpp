#include "DisarrayPCH.hpp"

#include "graphics/TextRenderer.hpp"

#include <ft2build.h>

#include "core/Log.hpp"
#include "core/filesystem/FileIO.hpp"
#include "graphics/Texture.hpp"
#include FT_FREETYPE_H

namespace Disarray {

struct TextRenderer::TextRenderingAPI {
	FT_Library library;
	std::vector<FT_Face> faces {};
};

template <> auto PimplDeleter<TextRenderer::TextRenderingAPI>::operator()(TextRenderer::TextRenderingAPI* ptr) noexcept -> void { delete ptr; }

void TextRenderer::construct(Disarray::Renderer& renderer, const Disarray::Device& device)
{
	renderer_api = make_scope<TextRenderingAPI, PimplDeleter<TextRenderingAPI>>();
	if (FT_Init_FreeType(&renderer_api->library)) {
		Log::error("TextRenderer", "Could not initialise FreeType.");
		throw;
	}

	FS::for_each_in_directory(
		std::filesystem::path { "Assets/Fonts" },
		[&dev = device, &characters = font_data, &fonts = renderer_api->faces, &library = renderer_api->library, index = 0U](
			const auto& entry) mutable {
			auto new_face = fonts.emplace_back();
			FT_New_Face(library, entry.path().string().c_str(), index++, &new_face);
			FT_Set_Pixel_Sizes(new_face, 0, 48);

			for (unsigned char c = 0; c < 128; c++) {
				// load character glyph
				if (FT_Load_Char(new_face, c, FT_LOAD_RENDER)) {
					std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
					continue;
				}

				DataBuffer bitmap_buffer { new_face->glyph->bitmap.buffer, new_face->glyph->bitmap.width * new_face->glyph->bitmap.rows };
				Log::info("TextRenderer", "Bitmap size is {} bytes", bitmap_buffer.get_size());
				auto texture = Texture::construct_scoped(dev,
					{
						.extent = { new_face->glyph->bitmap.width, new_face->glyph->bitmap.rows },
						.format = ImageFormat::Uint,
						.data_buffer = std::move(bitmap_buffer),
						.debug_name = fmt::format("glyph-{}", c),
					});

				// now store character for later use
				FontData& current = characters.at(static_cast<std::size_t>(c) + index * 128UL);
				current.texture = std::move(texture);
				current.size = glm::ivec2(new_face->glyph->bitmap.width, new_face->glyph->bitmap.rows);
				current.bearing = glm::ivec2(new_face->glyph->bitmap_left, new_face->glyph->bitmap_top);
				current.advance = new_face->glyph->advance.x;
			}
		},
		[](const std::filesystem::directory_entry& entry) { return entry.path().extension() == ".ttf"; });

	Log::info("TextRenderer", "Constructed {} faces!", renderer_api->faces.size());
}

void TextRenderer::submit_text(std::string_view text, const glm::uvec2&, float size) { }

void TextRenderer::render(Disarray::Renderer& renderer, Disarray::CommandExecutor& executor) { }

} // namespace Disarray
