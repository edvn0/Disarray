#include "DisarrayPCH.hpp"

#include "graphics/TextRenderer.hpp"

// clang-format off
#include <ft2build.h>
#include <vector>
#include "core/DataBuffer.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Pipeline.hpp"
#include FT_FREETYPE_H
// clang-format on

#include "core/Log.hpp"
#include "core/filesystem/FileIO.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/StorageBuffer.hpp"
#include "graphics/Texture.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/IndexBuffer.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/VertexBuffer.hpp"

namespace Disarray {

struct TextColourAndImage {
	glm::vec4 colour;
	alignas(16) std::uint32_t index;
};
static_assert(alignof(TextColourAndImage) == 16);
static_assert(sizeof(TextColourAndImage) == 32);

struct TextRenderer::TextRenderingAPI {
	FT_Library library;
	std::vector<FT_Face> faces {};

	Scope<Vulkan::IndexBuffer> glyph_ib;
	Scope<Vulkan::VertexBuffer> screen_space_glyph_vb;
	Scope<Vulkan::VertexBuffer> world_space_glyph_vb;
	Ref<Disarray::Pipeline> screen_space_glyph_pipeline;
	Ref<Disarray::Pipeline> world_space_glyph_pipeline;
	Ref<Disarray::Framebuffer> glyph_framebuffer;

	Scope<Disarray::StorageBuffer> colour_image_data;

	Disarray::Renderer* renderer { nullptr };
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
	renderer_api->screen_space_glyph_vb = make_scope<Vulkan::VertexBuffer>(device,
		BufferProperties {
			.size = screen_space_text_data.size() * sizeof(ScreenSpaceTextData),
			.count = screen_space_text_data.size(),
			.always_mapped = true,
		});
	renderer_api->world_space_glyph_vb = make_scope<Vulkan::VertexBuffer>(device,
		BufferProperties {
			.size = world_space_text_data.size() * sizeof(WorldSpaceTextData),
			.count = world_space_text_data.size(),
			.always_mapped = true,
		});

	renderer_api->colour_image_data = StorageBuffer::construct_scoped(device,
		{
			.size = world_space_text_data.size() * sizeof(TextColourAndImage),
			.count = world_space_text_data.size(),
			.always_mapped = true,
		});

	renderer.get_graphics_resource().expose_to_shaders(*renderer_api->colour_image_data, DescriptorSet { 3 }, DescriptorBinding { 6 });

	renderer_api->glyph_framebuffer = Framebuffer::construct(device,
		{
			.extent = extent,
			.attachments = { { ImageFormat::SBGR, true }, { ImageFormat::Depth } },
			.clear_colour_on_load = true,
			.clear_depth_on_load = true,
			.should_blend = true,
			.blend_mode = FramebufferBlendMode::SrcAlphaOneMinusSrcAlpha,
			.debug_name = "GlyphFramebuffer",
		});

	auto& resources = renderer.get_graphics_resource();
	const auto& desc_layout = resources.get_descriptor_set_layouts();

	renderer_api->screen_space_glyph_pipeline = Pipeline::construct(device,
		{
			.vertex_shader = renderer.get_pipeline_cache().get_shader("glyph_ss.vert"),
			.fragment_shader = renderer.get_pipeline_cache().get_shader("glyph_ss.frag"),
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

	renderer_api->world_space_glyph_pipeline = Pipeline::construct(device,
		{
			.vertex_shader = renderer.get_pipeline_cache().get_shader("glyph_ws.vert"),
			.fragment_shader = renderer.get_pipeline_cache().get_shader("glyph_ws.frag"),
			.framebuffer = renderer_api->glyph_framebuffer,
			.layout = {
				{ ElementType::Float3, "position" },
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

	renderer_api->renderer = &renderer;
	recreate(true, extent);
}

auto TextRenderer::recreate(bool, const Extent&) -> void
{
	auto& resources = renderer_api->renderer->get_graphics_resource();

	std::array<const Texture*, 128> textures {};
	std::size_t i = 0;
	for (const auto& tex : font_data) {
		textures.at(i++) = tex.texture == nullptr ? nullptr : tex.texture.get();
	}
	resources.expose_to_shaders(textures, DescriptorSet { 2 }, DescriptorBinding { 0 });
	resources.expose_to_shaders(renderer_api->glyph_framebuffer->get_image(0), DescriptorSet { 1 }, DescriptorBinding { 2 });
}

auto TextRenderer::get_pipelines() -> std::array<Disarray::Pipeline*, 2>
{
	return { renderer_api->world_space_glyph_pipeline.get(), renderer_api->screen_space_glyph_pipeline.get() };
}

void TextRenderer::submit_text(std::string_view text, const glm::uvec2& position, float scale, const glm::vec4& colour)
{
	ensure(screen_space_text_data_index < screen_space_text_data.size());
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

		screen_space_text_character_texture_data.at(screen_space_text_data_index) = static_cast<std::uint32_t>(character);
		screen_space_text_character_colour_data.at(screen_space_text_data_index) = colour;
		screen_space_text_data.at(screen_space_vertex_data_index++) = {
			.pos = { xpos, ypos },
			.tex_coords = { 0, 0 },
		};
		screen_space_text_data.at(screen_space_vertex_data_index++) = {
			.pos = { xpos, ypos + height },
			.tex_coords = { 0, 1 },
		};
		screen_space_text_data.at(screen_space_vertex_data_index++) = {
			.pos = { xpos + width, ypos + height },
			.tex_coords = { 1, 1 },
		};
		screen_space_text_data.at(screen_space_vertex_data_index++) = {
			.pos = { xpos + width, ypos },
			.tex_coords = { 1, 0 },
		};
		screen_space_text_data_index++;
		screen_space_submitted_vertices += 4;
	};
}

void TextRenderer::submit_text(std::string_view text, const glm::vec3& position, float scale, const glm::vec4& colour)
{
	ensure(world_space_text_data_index < world_space_text_data.size());
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

		world_space_text_character_texture_data.at(world_space_text_data_index) = static_cast<std::uint32_t>(character);
		world_space_text_character_colour_data.at(world_space_text_data_index) = colour;
		world_space_text_data.at(world_space_vertex_data_index++) = {
			.pos = { xpos, ypos, position.z },
			.tex_coords = { 0, 0 },
		};
		world_space_text_data.at(world_space_vertex_data_index++) = {
			.pos = { xpos, ypos + height, position.z },
			.tex_coords = { 0, 1 },
		};
		world_space_text_data.at(world_space_vertex_data_index++) = {
			.pos = { xpos + width, ypos + height, position.z },
			.tex_coords = { 1, 1 },
		};
		world_space_text_data.at(world_space_vertex_data_index++) = {
			.pos = { xpos + width, ypos, position.z },
			.tex_coords = { 1, 0 },
		};
		world_space_text_data_index++;
		world_space_submitted_vertices += 4;
	};
}

void TextRenderer::submit_text(std::string_view text, const glm::mat4& transform, float scale, const glm::vec4& colour)
{
	static constexpr auto rotate_3d_by_transform = [](const glm::vec3& vector, const glm::mat4& transform) {
		// Extract the rotation and scale components from the full matrix
		glm::mat4 copied = transform;

		// Set the translation components to identity (or zeros, depending on your convention)
		copied[3][0] = 0;
		copied[3][1] = 0;
		copied[3][2] = 0;
		copied[3][3] = 1;

		return glm::vec3(copied * glm::vec4(vector, 1.0F));
	};

	ensure(world_space_text_data_index < world_space_text_data.size());
	const auto& position = transform[3];

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

		world_space_text_character_texture_data.at(world_space_text_data_index) = static_cast<std::uint32_t>(character);
		world_space_text_character_colour_data.at(world_space_text_data_index) = colour;
		world_space_text_data.at(world_space_vertex_data_index++) = {
			.pos = rotate_3d_by_transform({ xpos, ypos, position.z }, transform),
			.tex_coords = { 0, 0 },
		};
		world_space_text_data.at(world_space_vertex_data_index++) = {
			.pos = rotate_3d_by_transform({ xpos, ypos + height, position.z }, transform),
			.tex_coords = { 0, 1 },
		};
		world_space_text_data.at(world_space_vertex_data_index++) = {
			.pos = rotate_3d_by_transform({ xpos + width, ypos + height, position.z }, transform),
			.tex_coords = { 1, 1 },
		};
		world_space_text_data.at(world_space_vertex_data_index++) = {
			.pos = rotate_3d_by_transform({ xpos + width, ypos, position.z }, transform),
			.tex_coords = { 1, 0 },
		};
		world_space_text_data_index++;
		world_space_submitted_vertices += 4;
	};
}

void TextRenderer::clear_pass(Disarray::Renderer& renderer, Disarray::CommandExecutor& executor)
{
	renderer.begin_pass(executor, *renderer_api->glyph_framebuffer, true);
	renderer.end_pass(executor);
}

void TextRenderer::render(Disarray::Renderer& renderer, Disarray::CommandExecutor& executor)
{
	renderer.begin_pass(executor, *renderer_api->glyph_framebuffer);
	{
		auto raw_pointer = renderer_api->colour_image_data->get_mutable<TextColourAndImage>();
		for (auto i = 0ULL; i < world_space_text_data_index; i++) {
			auto& current = raw_pointer[i];

			current.colour = world_space_text_character_colour_data.at(i);
			current.index = world_space_text_character_texture_data.at(i);
		}

		renderer_api->screen_space_glyph_vb->set_data(screen_space_text_data.data(), screen_space_vertex_data_index * sizeof(ScreenSpaceTextData));
		auto* cmd = supply_cast<Vulkan::CommandExecutor>(executor);
		const auto& vertex_buffer = renderer_api->screen_space_glyph_vb;

		const auto& vk_pipeline = cast_to<Vulkan::Pipeline>(*renderer_api->screen_space_glyph_pipeline);
		renderer.bind_pipeline(executor, *renderer_api->screen_space_glyph_pipeline);
		renderer.bind_descriptor_sets(executor, *renderer_api->screen_space_glyph_pipeline);

		const auto& index_buffer = renderer_api->glyph_ib;
		vkCmdBindIndexBuffer(cmd, supply_cast<Vulkan::IndexBuffer>(*index_buffer), 0, VK_INDEX_TYPE_UINT32);

		const std::array<VkBuffer, 1> vbs { supply_cast<Vulkan::VertexBuffer>(*vertex_buffer) };
		VkDeviceSize offsets { 0 };
		vkCmdBindVertexBuffers(cmd, 0, 1, vbs.data(), &offsets);

		vkCmdDrawIndexed(cmd, screen_space_text_data_index * 6, 1, 0, 0, 0);
	}

	{
		auto raw_pointer = renderer_api->colour_image_data->get_mutable<TextColourAndImage>();
		for (auto i = 0ULL; i < world_space_text_data_index; i++) {
			auto& current = raw_pointer[i];

			current.colour = world_space_text_character_colour_data.at(i);
			current.index = world_space_text_character_texture_data.at(i);
		}

		renderer_api->world_space_glyph_vb->set_data(world_space_text_data.data(), world_space_vertex_data_index * sizeof(WorldSpaceTextData));
		auto* cmd = supply_cast<Vulkan::CommandExecutor>(executor);
		const auto& vertex_buffer = renderer_api->world_space_glyph_vb;

		const auto& vk_pipeline = cast_to<Vulkan::Pipeline>(*renderer_api->world_space_glyph_pipeline);
		renderer.bind_pipeline(executor, *renderer_api->world_space_glyph_pipeline);
		renderer.bind_descriptor_sets(executor, *renderer_api->world_space_glyph_pipeline);

		const auto& index_buffer = renderer_api->glyph_ib;
		vkCmdBindIndexBuffer(cmd, supply_cast<Vulkan::IndexBuffer>(*index_buffer), 0, VK_INDEX_TYPE_UINT32);

		const std::array<VkBuffer, 1> vbs { supply_cast<Vulkan::VertexBuffer>(*vertex_buffer) };
		VkDeviceSize offsets { 0 };
		vkCmdBindVertexBuffers(cmd, 0, 1, vbs.data(), &offsets);

		auto& push_constant = renderer.get_graphics_resource().get_editable_push_constant();
		vkCmdDrawIndexed(cmd, world_space_text_data_index * 6, 1, 0, 0, 0);
	}

	screen_space_text_data_index = 0;
	screen_space_vertex_data_index = 0;
	screen_space_submitted_vertices = 0;
	world_space_text_data_index = 0;
	world_space_vertex_data_index = 0;
	world_space_submitted_vertices = 0;

	renderer.end_pass(executor);
}

} // namespace Disarray
