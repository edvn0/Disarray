#include "DisarrayPCH.hpp"

#include "graphics/Renderer.hpp"

#include <array>

#include "core/Types.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Image.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Swapchain.hpp"
#include "vulkan/Framebuffer.hpp"
#include "vulkan/GraphicsResource.hpp"
#include "vulkan/Renderer.hpp"

namespace Disarray::Vulkan {

Renderer::Renderer(const Disarray::Device& dev, const Disarray::Swapchain& sc, const Disarray::RendererProperties& properties)
	: Disarray::Renderer(make_scope<GraphicsResource>(dev, sc))
	, device(dev)
	, swapchain(sc)
	, props(properties)
	, extent(swapchain.get_extent())
{
	FramebufferProperties geometry_props {
		.extent = swapchain.get_extent(),
		.attachments = { { ImageFormat::SBGR }, { ImageFormat::Depth } },
		.clear_colour_on_load = false,
		.clear_depth_on_load = false,
		.debug_name = "RendererFramebuffer",
	};
	geometry_framebuffer = Framebuffer::construct(device, geometry_props);

	quad_framebuffer = Framebuffer::construct(device,
			{
				.extent = extent,
				.attachments = { { ImageFormat::SBGR }, { ImageFormat::Uint, false }, { ImageFormat::Depth }, },
				.debug_name = "QuadFramebuffer",
			});

	PipelineCacheCreationProperties pipeline_properties = {
		.pipeline_key = "quad",
		.vertex_shader_key = "quad.vert",
		.fragment_shader_key = "quad.frag",
		.framebuffer = geometry_framebuffer,
		.layout = { { ElementType::Float3, "position" }, { ElementType::Float2, "uvs" }, { ElementType::Float3, "normals" },
			{ ElementType::Float4, "colour" }, { ElementType::Uint, "identifier" }, },
		.push_constant_layout = PushConstantLayout { PushConstantRange { PushConstantKind::Both, sizeof(PushConstant) } },
		.extent = swapchain.get_extent(),
		.descriptor_set_layouts = get_graphics_resource().get_descriptor_set_layouts(),
	};

	auto& resource = get_graphics_resource();
	{
		// Quad
		pipeline_properties.framebuffer = quad_framebuffer;

		resource.get_pipeline_cache().put(pipeline_properties);
	}
	{
		// Line
		pipeline_properties.framebuffer = geometry_framebuffer;
		pipeline_properties.pipeline_key = "line";
		pipeline_properties.vertex_shader_key = "line.vert";
		pipeline_properties.fragment_shader_key = "line.frag";
		pipeline_properties.write_depth = false;
		pipeline_properties.test_depth = false;
		pipeline_properties.line_width = 3.0F;
		pipeline_properties.polygon_mode = PolygonMode::Line;
		pipeline_properties.layout = { { ElementType::Float3, "pos" }, { ElementType::Float4, "colour" } };
		resource.get_pipeline_cache().put(pipeline_properties);
	}
	{
		// Line
		pipeline_properties.framebuffer = quad_framebuffer;
		pipeline_properties.pipeline_key = "line_id";
		pipeline_properties.vertex_shader_key = "line_id.vert";
		pipeline_properties.fragment_shader_key = "line_id.frag";
		pipeline_properties.layout = { { ElementType::Float3, "pos" }, { ElementType::Float4, "colour" }, { ElementType::Uint, "id" } };
		resource.get_pipeline_cache().put(pipeline_properties);
	}

	batch_renderer.construct(*this, device);
	text_renderer.construct(*this, device, extent);

	{
		fullscreen_framebuffer = Framebuffer::construct(device,
			{
				.extent = extent,
				.attachments = { { ImageFormat::SBGR } },
				.debug_name = "FullscreenFramebuffer",
			});
		fullscreen_quad_pipeline = Pipeline::construct_scoped(device,
			PipelineProperties {
				.vertex_shader = get_graphics_resource().get_pipeline_cache().get_shader("fullscreen_quad.vert"),
				.fragment_shader = get_graphics_resource().get_pipeline_cache().get_shader("fullscreen_quad.frag"),
				.framebuffer = fullscreen_framebuffer,
				.push_constant_layout = { { PushConstantKind::Both, sizeof(PushConstant) } },
				.extent = extent,
				.cull_mode = CullMode::Front,
				.write_depth = false,
				.test_depth = false,
				.descriptor_set_layouts = get_graphics_resource().get_descriptor_set_layouts(),
			});
	}
}

Renderer::~Renderer() = default;

void Renderer::on_resize()
{
	extent = swapchain.get_extent();
	get_graphics_resource().recreate(true, extent);

	get_texture_cache().force_recreate(extent);

	get_pipeline_cache().for_each_in_storage([new_descs = get_graphics_resource().get_descriptor_set_layouts()](auto&& key_value_pair) {
		auto& [key, pipeline] = key_value_pair;
		pipeline->get_properties().descriptor_set_layouts = new_descs;
	});
	get_pipeline_cache().force_recreate(extent);

	geometry_framebuffer->recreate(true, extent);
	quad_framebuffer->recreate(true, extent);
}

void Renderer::begin_frame(const Camera& camera)
{
	auto [ubo, camera_ubo, lights, _, __, ___] = get_graphics_resource().get_editable_ubos();
	camera_ubo.position = glm::vec4 { camera.get_position(), 1.0F };
	camera_ubo.direction = glm::vec4 { camera.get_direction(), 1.0F };

	begin_frame(camera.get_view_matrix(), camera.get_projection_matrix(), camera.get_view_projection());
}

void Renderer::begin_frame(const glm::mat4& view, const glm::mat4& proj, const glm::mat4& view_projection)
{
	// TODO: Move to some kind of scene scope?
	batch_renderer.reset();

	auto [ubo, camera, lights, _, __, ___] = get_graphics_resource().get_editable_ubos();

	ubo.view = view;
	ubo.proj = proj;
	ubo.view_projection = view_projection;

	if (swapchain.needs_recreation()) {
		force_recreation();
	}
}

void Renderer::end_frame()
{
	auto [ubo, camera_ubo, lights, shadow_pass, directional, glyph] = get_graphics_resource().get_editable_ubos();

	ubo.reset();
	camera_ubo.reset();
	lights.reset();
	shadow_pass.reset();
	directional.reset();
	glyph.reset();
}

void Renderer::force_recreation() { on_resize(); }

void Renderer::submit_batched_geometry(Disarray::CommandExecutor& executor)
{
	if (batch_renderer.should_submit()) {
		batch_renderer.submit(*this, executor);
	}

	batch_renderer.reset();
}

void Renderer::draw_text(std::string_view text, const glm::uvec2& position, float size) { text_renderer.submit_text(text, position, size); }

void Renderer::draw_planar_geometry(Geometry geometry, const GeometryProperties& properties)
{
	if (batch_renderer.would_be_full()) {
		on_batch_full_func(*this);
	}
	add_geometry_to_batch(geometry, properties);
}

void Renderer::add_geometry_to_batch(Disarray::Geometry geometry, const Disarray::GeometryProperties& properties)
{
	batch_renderer.create_new(geometry, properties);
	batch_renderer.submitted_geometries++;
}

void Renderer::flush_batch(Disarray::CommandExecutor& executor) { batch_renderer.flush(*this, executor); }

auto Renderer::get_composite_pass_image() const -> const Disarray::Image& { return fullscreen_framebuffer->get_image(); }

} // namespace Disarray::Vulkan
