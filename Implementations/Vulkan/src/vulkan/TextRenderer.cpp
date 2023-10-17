#include "DisarrayPCH.hpp"

#include "graphics/TextRenderer.hpp"

// clang-format off
#include <ft2build.h>
#include <vector>
#include "core/DataBuffer.hpp"
#include "graphics/Pipeline.hpp"
#include FT_FREETYPE_H
// clang-format on

#include "core/Log.hpp"
#include "core/filesystem/FileIO.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Texture.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/IndexBuffer.hpp"
#include "vulkan/Pipeline.hpp"
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

template <> auto PimplDeleter<TextRenderer::TextRenderingAPI>::operator()(TextRenderer::TextRenderingAPI* ptr) noexcept -> void
{
	FT_Done_FreeType(ptr->library);
	delete ptr;
}

void TextRenderer::construct(Disarray::Renderer& renderer, const Disarray::Device& device, const Extent& extent)
{
	renderer_api = make_scope<TextRenderingAPI, PimplDeleter<TextRenderingAPI>>();
	if (FT_Init_FreeType(&renderer_api->library) != FT_Error { 0 }) {
		Log::error("TextRenderer", "Could not initialise FreeType.");
		throw;
	}

	FS::for_each_in_directory(std::filesystem::path { "Assets/Fonts" },
		[&max_h = max_height, &dev = device, &characters = font_data, &fonts = renderer_api->faces, &library = renderer_api->library, index = 0U](
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
					Log::error("TextRenderer", "Could not load character {} bitmap data. Size was {}, {}. Buffer had pointer {}", char_code,
						new_face->glyph->bitmap.width, new_face->glyph->bitmap.rows, fmt::ptr(new_face->glyph->bitmap.buffer));
				}

				static constexpr auto one_if_leq = [](const auto& value) { return value <= 0 ? 1 : value; };
				static std::array<std::uint8_t, 1> empty_data { 0 };

				Scope<Disarray::Texture> texture = nullptr;
				texture = Texture::construct_scoped(dev,
					{
						.extent = { one_if_leq(new_face->glyph->bitmap.width), one_if_leq(new_face->glyph->bitmap.rows) },
						.format = ImageFormat::Red,
						.data_buffer = bitmap_buffer.is_valid() ? std::move(bitmap_buffer) : DataBuffer { empty_data.data(), empty_data.size() },
						.debug_name = fmt::format("glyph-{}", char_code),
					});

				// now store character for later use
				FontData& current = characters.at(static_cast<std::size_t>(char_code) + index * characters.size());
				current.texture = std::move(texture);
				current.size = glm::ivec2(new_face->glyph->bitmap.width, new_face->glyph->bitmap.rows);
				current.bearing = glm::ivec2(new_face->glyph->bitmap_left, new_face->glyph->bitmap_top);
				current.advance = new_face->glyph->advance.x;

				if (const auto height = new_face->glyph->bitmap.rows; height > max_h) {
					max_h = height;
				}
			}
			FT_Done_Face(new_face);
			index++;
		},
		{ ".ttf" });

	Log::info("TextRenderer", "Constructed {} faces!", renderer_api->faces.size());

	auto glyph_quad_vb = VertexBuffer::construct(device,
		{
			.size = text_data.size() * sizeof(TextData),
			.count = text_data.size(),
		});
	static constexpr auto index_count = 6;
	std::array<std::uint32_t, glyph_count * index_count> index_buffer {};
	std::uint32_t offset = 0;
	for (std::size_t i = 0; i < index_buffer.size(); i += index_count) {
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
			.size = text_data.size() * sizeof(TextData),
			.count = text_data.size(),
		});

	renderer_api->glyph_framebuffer = Framebuffer::construct(device,
		{
			.extent = extent,
			.attachments = { { ImageFormat::SBGR, true } },
			.should_blend = true,
			.blend_mode = FramebufferBlendMode::SrcAlphaOneMinusSrcAlpha,
			.debug_name = "GlyphFramebuffer",
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
			.cull_mode = CullMode::Back,
			.face_mode = FaceMode::CounterClockwise,
			.write_depth = false,
			.test_depth = false,
			.descriptor_set_layouts = desc_layout,
		});

	std::array<const Texture*, 128> textures {};
	std::size_t i = 0;
	for (const auto& tex : font_data) {
		textures.at(i++) = tex.texture == nullptr ? nullptr : tex.texture.get();
	}
	resources.expose_to_shaders(textures, 2, 2);
	resources.expose_to_shaders(renderer_api->glyph_framebuffer->get_image(0), 1, 2);
}

void TextRenderer::submit_text(std::string_view text, const glm::uvec2& position, float scale)
{
	ensure(text_data_index < text_data.size());
	auto x_position = static_cast<float>(position.x);
	static auto max_h = static_cast<float>(max_height);
	auto y_position = static_cast<float>(position.y) + max_h;
	for (const auto& character : text) {
		if (character == '\n' || character == '\r') {
			y_position += max_h;
			x_position = static_cast<float>(position.x);
			continue;
		}
		auto& character_font_data = font_data.at(static_cast<std::size_t>(character));
		float xpos = x_position + static_cast<float>(character_font_data.bearing.x) * scale;
		float ypos = y_position - static_cast<float>(character_font_data.bearing.y) * scale;

		float width = static_cast<float>(character_font_data.size.x) * scale;
		float height = static_cast<float>(character_font_data.size.y) * scale;
		auto scaled_advance = static_cast<float>((character_font_data.advance >> 6)) * scale;
		x_position += scaled_advance;

		text_character_texture_data.at(text_data_index) = static_cast<std::uint32_t>(character);
		text_data.at(vertex_data_index++) = {
			.pos = { xpos, ypos },
			.tex_coords = { 0, 0 },
		};
		text_data.at(vertex_data_index++) = {
			.pos = { xpos, ypos + height },
			.tex_coords = { 0, 1 },
		};
		text_data.at(vertex_data_index++) = {
			.pos = { xpos + width, ypos + height },
			.tex_coords = { 1, 1 },
		};
		text_data.at(vertex_data_index++) = {
			.pos = { xpos + width, ypos },
			.tex_coords = { 1, 0 },
		};
		text_data_index++;
		submitted_vertices += 4;
	};
}

void TextRenderer::render(Disarray::Renderer& renderer, Disarray::CommandExecutor& executor)
{
	renderer.begin_pass(executor, *renderer_api->glyph_framebuffer);
	auto&& ubos = renderer.get_graphics_resource().get_editable_ubos();

	static auto extent = renderer_api->glyph_pipeline->get_properties().extent.as<float>();
	static auto ortho = glm::ortho(0.F, extent.width, 0.F, extent.height);
	auto& glyph_ubo = std::get<GlyphUBO&>(ubos);
	glyph_ubo.projection = ortho;
	renderer.get_graphics_resource().update_ubo(UBOIdentifier::Glyph);

	renderer_api->glyph_vb->set_data(text_data.data(), vertex_data_index * sizeof(TextData));
	auto* cmd = supply_cast<Vulkan::CommandExecutor>(executor);
	const auto& vertex_buffer = renderer_api->glyph_vb;

	const auto& vk_pipeline = cast_to<Vulkan::Pipeline>(*renderer_api->glyph_pipeline);
	renderer.bind_pipeline(executor, *renderer_api->glyph_pipeline);

	const std::array desc { renderer.get_graphics_resource().get_descriptor_set(0), renderer.get_graphics_resource().get_descriptor_set(1),
		renderer.get_graphics_resource().get_descriptor_set(2) };
	vkCmdBindDescriptorSets(
		cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline.get_layout(), 0, static_cast<std::uint32_t>(desc.size()), desc.data(), 0, nullptr);

	const auto& index_buffer = renderer_api->glyph_ib;
	vkCmdBindIndexBuffer(cmd, supply_cast<Vulkan::IndexBuffer>(*index_buffer), 0, VK_INDEX_TYPE_UINT32);

	const std::array<VkBuffer, 1> vbs { supply_cast<Vulkan::VertexBuffer>(*vertex_buffer) };
	VkDeviceSize offsets { 0 };
	vkCmdBindVertexBuffers(cmd, 0, 1, vbs.data(), &offsets);

	auto& push_constant = renderer.get_graphics_resource().get_editable_push_constant();
	for (auto i = 0U; i < text_data_index; i++) {
		push_constant.image_indices[0] = static_cast<std::int32_t>(text_character_texture_data.at(i));
		vkCmdPushConstants(cmd, vk_pipeline.get_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant),
			renderer.get_graphics_resource().get_push_constant());

		vkCmdDrawIndexed(cmd, 6, 1, i * 6, 0, 0);
	}

	text_data_index = 0;
	vertex_data_index = 0;
	submitted_vertices = 0;
	text_data.fill({});
	text_character_texture_data.fill({});

	renderer.end_pass(executor, false);
}

} // namespace Disarray
