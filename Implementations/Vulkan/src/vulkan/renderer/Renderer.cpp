#include "DisarrayPCH.hpp"

#include <vulkan/vulkan_core.h>

#include <array>
#include <future>
#include <thread>

#include "core/Collections.hpp"
#include "core/Types.hpp"
#include "core/filesystem/AssetLocations.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Image.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Swapchain.hpp"
#include "util/Timer.hpp"
#include "vulkan/Device.hpp"
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
}

Renderer::~Renderer() = default;

void Renderer::construct_sub_renderers(const Disarray::Device&, Disarray::App& app)
{
	batch_renderer.construct(*this, device);
	text_renderer.construct(*this, device, extent);
}

void Renderer::on_resize()
{
	extent = swapchain.get_extent();
	get_graphics_resource().recreate(true, extent);
	get_texture_cache().force_recreation(extent);
	get_pipeline_cache().for_each_in_storage([new_descs = get_graphics_resource().get_descriptor_set_layouts()](auto&& key_value_pair) {
		auto&& [key, pipeline] = key_value_pair;
		pipeline->get_properties().descriptor_set_layouts = new_descs;
	});
	get_pipeline_cache().force_recreation(extent);
	text_renderer.recreate(true, extent);
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
	// TODO(edvin): Move to some kind of scene scope?
	batch_renderer.reset();

	auto [ubo, camera, lights, _, __, ___] = get_graphics_resource().get_editable_ubos();
	// TODO(edvin): this may be incorrect!
	const auto view_matrix = glm::inverse(view);
	camera.position = view_matrix[3];
	camera.direction = -view_matrix[2];
	ubo.view = view;
	ubo.proj = proj;
	ubo.view_projection = view_projection;

	camera.view = view;

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

void Renderer::clear_pass(Disarray::CommandExecutor& executor, RenderPasses pass)
{
	switch (pass) {
	case RenderPasses::Text:
		text_renderer.clear_pass(*this, executor);
		break;
	case RenderPasses::PlanarGeometry:
		batch_renderer.clear_pass(*this, executor);
		break;
	}
}

void Renderer::clear_pass(Disarray::CommandExecutor& executor, Disarray::Framebuffer& pass)
{
	BaseRenderer::begin_pass(executor, pass, true);
	end_pass(executor);
}

void Renderer::submit_batched_geometry(Disarray::CommandExecutor& executor)
{
	if (batch_renderer.should_submit()) {
		batch_renderer.submit(*this, executor);
	}

	batch_renderer.reset();
}

void Renderer::draw_text(std::string_view text, const glm::mat4& transform, float size, const glm::vec4& colour)
{
	text_renderer.submit_text(text, transform, size, colour);
}

void Renderer::draw_text(std::string_view text, const glm::uvec2& position, float size, const glm::vec4& colour)
{
	text_renderer.submit_text(text, position, size, colour);
}

void Renderer::draw_text(std::string_view text, const glm::vec3& position, float size, const glm::vec4& colour)
{
	text_renderer.submit_text(text, position, size, colour);
}

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

} // namespace Disarray::Vulkan
