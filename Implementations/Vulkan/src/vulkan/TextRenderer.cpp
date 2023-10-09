#include "DisarrayPCH.hpp"

#include "graphics/TextRenderer.hpp"

// clang-format off
#include <ft2build.h>
#include FT_FREETYPE_H
// clang-format on

#include "core/Log.hpp"
#include "core/filesystem/FileIO.hpp"
#include "graphics/Texture.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/GraphicsResource.hpp"
#include "vulkan/IndexBuffer.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/Renderer.hpp"
#include "vulkan/VertexBuffer.hpp"

namespace Disarray {

struct TextRenderer::TextRenderingAPI {
	FT_Library library;
	std::vector<FT_Face> faces {};

	Scope<Vulkan::IndexBuffer> glyph_ib;
	Scope<Vulkan::VertexBuffer> glyph_vb;
	Ref<Disarray::Pipeline> glyph_pipeline;
	Ref<Disarray::Framebuffer> glyph_framebuffer;
};

template <> auto PimplDeleter<TextRenderer::TextRenderingAPI>::operator()(TextRenderer::TextRenderingAPI* ptr) noexcept -> void { delete ptr; }

void TextRenderer::construct(Disarray::Renderer& renderer, const Disarray::Device& device, const Extent& extent)
{
	renderer_api = make_scope<TextRenderingAPI, PimplDeleter<TextRenderingAPI>>();
	if (FT_Init_FreeType(&renderer_api->library) != FT_Error { 0 }) {
		Log::error("TextRenderer", "Could not initialise FreeType.");
		throw;
	}

	FS::for_each_in_directory(
		std::filesystem::path { "Assets/Fonts" },
		[&dev = device, &characters = font_data, &fonts = renderer_api->faces, &library = renderer_api->library, index = 0U](
			const auto& entry) mutable {
			auto& new_face = fonts.emplace_back();
			FT_New_Face(library, entry.path().string().c_str(), index, &new_face);
			FT_Set_Pixel_Sizes(new_face, 0, 48);

			for (unsigned char char_code = 0; char_code < characters.size(); char_code++) {
				// load character glyph
				if (FT_Load_Char(new_face, char_code, FT_LOAD_RENDER)) {
					Log::error("TextRenderer", "Could not load character {}", char_code);
					continue;
				}

				DataBuffer bitmap_buffer { new_face->glyph->bitmap.buffer, new_face->glyph->bitmap.width * new_face->glyph->bitmap.rows };

				if (!bitmap_buffer.is_valid()) {
					Log::error("TextRenderer", "Could not load character {} bitmap data.", char_code);
					continue;
				}

				auto texture = Texture::construct_scoped(dev,
					{
						.extent = { new_face->glyph->bitmap.width, new_face->glyph->bitmap.rows },
						.format = ImageFormat::Red,
						.data_buffer = std::move(bitmap_buffer),
						.debug_name = fmt::format("glyph-{}", char_code),
					});

				// now store character for later use
				FontData& current = characters.at(static_cast<std::size_t>(char_code) + index * 128UL);
				current.texture = std::move(texture);
				current.size = glm::ivec2(new_face->glyph->bitmap.width, new_face->glyph->bitmap.rows);
				current.bearing = glm::ivec2(new_face->glyph->bitmap_left, new_face->glyph->bitmap_top);
				current.advance = new_face->glyph->advance.x;
			}
			index++;
		},
		[](const std::filesystem::directory_entry& entry) { return entry.path().extension() == ".ttf"; });

	Log::info("TextRenderer", "Constructed {} faces!", renderer_api->faces.size());

	auto glyph_quad_vb = VertexBuffer::construct(device,
		{
			.size = text_data.size() * 4 * sizeof(TextData::VertexData),
			.count = text_data.size(),
		});
	std::array<std::uint32_t, 1000 * 6> index_buffer {};
	std::uint32_t offset = 0;
	for (std::size_t i = 0; i < index_buffer.size(); i += 6) {
		index_buffer.at(i + 0) = 0 + offset;
		index_buffer.at(i + 1) = 1 + offset;
		index_buffer.at(i + 2) = 2 + offset;
		index_buffer.at(i + 3) = 2 + offset;
		index_buffer.at(i + 4) = 3 + offset;
		index_buffer.at(i + 5) = 0 + offset;
		offset += 4;
	}
	renderer_api->glyph_ib = make_scope<Vulkan::IndexBuffer>(device,
		BufferProperties {
			.data = index_buffer.data(),
			.size = index_buffer.size() * sizeof(std::uint32_t),
			.count = index_buffer.size(),
		});
	renderer_api->glyph_vb = make_scope<Vulkan::VertexBuffer>(device,
		BufferProperties {
			.size = text_data.size() * 4 * sizeof(TextData::VertexData),
			.count = text_data.size(),
		});

	renderer_api->glyph_framebuffer = Framebuffer::construct(device,
		{
			.extent = extent,
			.attachments = { { ImageFormat::SBGR, true } },
			.should_blend = true,
			.blend_mode = FramebufferBlendMode::SrcAlphaOneMinusSrcAlpha,
			.debug_name = "Glyph",
		});

	auto& resources = renderer.get_graphics_resource();
	const auto& desc_layout = resources.get_descriptor_set_layouts();

	renderer_api->glyph_pipeline = Pipeline::construct(device,
		{
			.vertex_shader = renderer.get_pipeline_cache().get_shader("glyph.vert"),
			.fragment_shader = renderer.get_pipeline_cache().get_shader("glyph.frag"),
			.framebuffer = renderer_api->glyph_framebuffer,
			.layout = {
				{ ElementType::Float2, "position" },
				{ ElementType::Float2, "tex_coords" },
			},
			.push_constant_layout = { { PushConstantKind::Both, sizeof(PushConstant) } },
			.extent = extent,
			.cull_mode = CullMode::Front,
			.write_depth = false,
			.test_depth = false,
			.descriptor_set_layouts = desc_layout,
		});

	std::array<const Texture*, 127> textures {};
	std::size_t i = 0;
	for (const auto& tex : font_data) {
		if (tex.texture == nullptr)
			continue;
		textures.at(i++) = tex.texture.get();
	}
	resources.expose_to_shaders(textures, 2);
}

void TextRenderer::submit_text(std::string_view text, const glm::uvec2& position, float scale)
{
	auto x = position.x;
	const auto& y = position.y;
	for (const auto& character : text) {
		auto& ch = font_data.at(static_cast<std::size_t>(character));
		float xpos = x + ch.bearing.x * scale;
		float ypos = y - (ch.size.y - ch.bearing.y) * scale;

		float w = ch.size.x * scale;
		float h = ch.size.y * scale;
		x += (ch.advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)

		auto& defered = text_data.at(text_data_index++);
		defered.character_texture_index = static_cast<std::size_t>(character);
		defered.character = character;
		defered.glyph_width = w;
		defered.glyph_height = h;
		defered.position = { xpos, ypos };
		defered.vertices = {
			TextData::VertexData {
				.pos = { xpos, ypos + h },
				.tex_coords = { 0, 0 },
			},
			{
				.pos = { xpos, ypos },
				.tex_coords = { 0, 1 },
			},
			{
				.pos = { xpos + w, ypos },
				.tex_coords = { 1, 1 },
			},
			{
				.pos = { xpos + w, ypos + h },
				.tex_coords = { 1, 0 },
			},
		};
	}
}

void TextRenderer::render(Disarray::Renderer& renderer, Disarray::CommandExecutor& executor)
{
	renderer.begin_pass(executor, *renderer_api->glyph_framebuffer);
	auto&& ubos = renderer.get_graphics_resource().get_editable_ubos();

	static auto extent = renderer_api->glyph_pipeline->get_properties().extent.as<float>();
	static auto ortho = glm::ortho(-extent.width / 2, extent.width / 2, -extent.height / 2, extent.height / 2);
	std::get<GlyphUBO&>(ubos).projection = ortho;
	renderer.get_graphics_resource().update_ubo(5);

	renderer_api->glyph_vb->set_data(text_data.data(), text_data_index * sizeof(TextData::VertexData));
	auto* cmd = supply_cast<Vulkan::CommandExecutor>(executor);

	const auto& vb = renderer_api->glyph_vb;
	const auto& ib = renderer_api->glyph_ib;
	const auto index_count = text_data_index * 6;
	const auto& vk_pipeline = cast_to<Vulkan::Pipeline>(*renderer_api->glyph_pipeline);

	const std::array<VkDescriptorSet, 1> desc { renderer.get_graphics_resource().get_descriptor_set() };
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline.get_layout(), 0, 1, desc.data(), 0, nullptr);

	renderer.bind_pipeline(executor, *renderer_api->glyph_pipeline);

	vkCmdPushConstants(cmd, vk_pipeline.get_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant),
		renderer.get_graphics_resource().get_push_constant());

	const std::array<VkBuffer, 1> vbs { supply_cast<Vulkan::VertexBuffer>(*vb) };
	constexpr VkDeviceSize offsets { 0 };
	vkCmdBindVertexBuffers(cmd, 0, 1, vbs.data(), &offsets);

	vkCmdBindIndexBuffer(cmd, supply_cast<Vulkan::IndexBuffer>(*ib), 0, VK_INDEX_TYPE_UINT32);

	const auto count = static_cast<std::uint32_t>(index_count);
	vkCmdDrawIndexed(cmd, count, 1, 0, 0, 0);
	text_data_index = 0;
	text_data.fill({});
	renderer.end_pass(executor, false);
}

} // namespace Disarray
