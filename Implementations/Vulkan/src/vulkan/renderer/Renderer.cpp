#include "DisarrayPCH.hpp"

#include "graphics/Renderer.hpp"

#include <array>

#include "core/Collections.hpp"
#include "core/Types.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Swapchain.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Framebuffer.hpp"
#include "vulkan/GraphicsResource.hpp"
#include "vulkan/IndexBuffer.hpp"
#include "vulkan/Mesh.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/Renderer.hpp"
#include "vulkan/UniformBuffer.hpp"

namespace Disarray::Vulkan {

Renderer::Renderer(const Disarray::Device& dev, const Disarray::Swapchain& sc, const Disarray::RendererProperties& properties)
	: Disarray::Renderer(std::make_unique<GraphicsResource>(dev, sc))
	, device(dev)
	, swapchain(sc)
	, props(properties)
	, extent(swapchain.get_extent())
{
	FramebufferProperties geometry_props { .extent = swapchain.get_extent(),
		.attachments = { { ImageFormat::SBGR }, { ImageFormat::Depth } },
		.clear_colour_on_load = false,
		.clear_depth_on_load = false,
		.debug_name = "RendererFramebuffer" };
	geometry_framebuffer = Framebuffer::construct(device, geometry_props);

	quad_framebuffer = Framebuffer::construct(device,
			{ .extent = swapchain.get_extent(),
				.attachments = { { ImageFormat::SBGR },  { ImageFormat::Uint, false },{ ImageFormat::Depth }, },
				.debug_name = "QuadFramebuffer" });

	PipelineCacheCreationProperties pipeline_properties = {
		.pipeline_key = "quad",
		.vertex_shader_key = "quad.vert",
		.fragment_shader_key = "quad.frag",
		.framebuffer = geometry_framebuffer,
		.layout = { { ElementType::Float3, "position" }, { ElementType::Float2, "uvs" }, { ElementType::Float3, "normals" },
			{ ElementType::Float4, "colour" }, { ElementType::Uint, "identifier" } },
		.push_constant_layout = PushConstantLayout { PushConstantRange { PushConstantKind::Both, sizeof(PushConstant) } },
		.extent = swapchain.get_extent(),
		.descriptor_set_layouts = get_graphics_resource().get_descriptor_set_layouts(),
	};
	{
		// Quad
		pipeline_properties.framebuffer = quad_framebuffer;
		get_pipeline_cache().put(pipeline_properties);
	}
	{
		// Line
		pipeline_properties.framebuffer = geometry_framebuffer;
		pipeline_properties.pipeline_key = "line";
		pipeline_properties.vertex_shader_key = "line.vert";
		pipeline_properties.fragment_shader_key = "line.frag";
		pipeline_properties.line_width = 8.0f;
		pipeline_properties.polygon_mode = PolygonMode::Line;
		pipeline_properties.layout = { { ElementType::Float3, "pos" }, { ElementType::Float4, "colour" } };
		get_pipeline_cache().put(pipeline_properties);
	}
	{
		// Line
		pipeline_properties.framebuffer = quad_framebuffer;
		pipeline_properties.pipeline_key = "line_id";
		pipeline_properties.vertex_shader_key = "line_id.vert";
		pipeline_properties.fragment_shader_key = "line_id.frag";
		pipeline_properties.layout = { { ElementType::Float3, "pos" }, { ElementType::Float4, "colour" }, { ElementType::Uint, "id" } };
		get_pipeline_cache().put(pipeline_properties);
	}

	batch_renderer.construct(*this, device);
}

Renderer::~Renderer() = default;

void Renderer::on_resize()
{
	extent = swapchain.get_extent();
	geometry_framebuffer->recreate(true, extent);
	quad_framebuffer->recreate(true, extent);
	get_texture_cache().force_recreate(extent);
	get_pipeline_cache().force_recreate(extent);
}

void Renderer::begin_frame(const Camera& camera)
{
	begin_frame(camera.get_view_matrix(), camera.get_projection_matrix(), camera.get_view_projection());
}

void Renderer::begin_frame(const glm::mat4& view, const glm::mat4& proj, const glm::mat4& view_projection)
{
	// TODO: Move to some kind of scene scope?
	batch_renderer.reset();

	auto& ubo = get_graphics_resource().get_editable_ubo();

	ubo.view = view;
	ubo.proj = proj;
	ubo.view_projection = view_projection;

	get_graphics_resource().update_ubo();

	if (swapchain.needs_recreation()) {
		force_recreation();
	}
}

void Renderer::end_frame() { std::memset(&get_graphics_resource().get_editable_ubo(), 0, sizeof(UBO)); }

void Renderer::force_recreation() { on_resize(); }

void Renderer::submit_batched_geometry(Disarray::CommandExecutor& executor)
{
	if (batch_renderer.should_submit()) {
		batch_renderer.submit(*this, executor);
	}

	batch_renderer.reset();
}

void Renderer::draw_planar_geometry(Geometry geometry, const GeometryProperties& properties)
{
	if (batch_renderer.would_be_full()) {
		add_geometry_to_batch(geometry, properties);
		on_batch_full_func(*this);
	} else {
		add_geometry_to_batch(geometry, properties);
	}
}

void Renderer::add_geometry_to_batch(Disarray::Geometry geometry, const Disarray::GeometryProperties& properties)
{
	batch_renderer.create_new(geometry, properties);
	batch_renderer.submitted_geometries++;
}

void Renderer::flush_batch(Disarray::CommandExecutor& executor) { batch_renderer.flush(*this, executor); }

} // namespace Disarray::Vulkan
