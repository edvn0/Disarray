#include "DisarrayPCH.hpp"

#include <core/filesystem/AssetLocations.hpp>
#include <graphics/Mesh.hpp>
#include <ui/UI.hpp>

#include "core/App.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/GLM.hpp"
#include "graphics/Maths.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/PushConstantLayout.hpp"
#include "graphics/RenderBatch.hpp"
#include "graphics/RenderPass.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/RendererProperties.hpp"
#include "graphics/ShaderCompiler.hpp"
#include "graphics/StorageBuffer.hpp"
#include "graphics/Swapchain.hpp"
#include "graphics/Texture.hpp"
#include "graphics/TextureCache.hpp"
#include "scene/SceneRenderer.hpp"

namespace Disarray {

SceneRenderer::SceneRenderer(const Disarray::Device& dev)
	: device(dev)
{
}

auto SceneRenderer::construct(Disarray::App& app) -> void
{
	renderer_extent = app.get_swapchain().get_extent();
	renderer = Renderer::construct_unique(device, app.get_swapchain(), {});
	command_executor = CommandExecutor::construct(device, &app.get_swapchain(), { .count = 3, .is_primary = true, .record_stats = true });

	renderer->on_batch_full([&exec = *command_executor](Renderer& ren) { ren.flush_batch(exec); });

	auto& resources = get_graphics_resource();
	const auto& desc_layout = resources.get_descriptor_set_layouts();
	auto temporary_shadow = Framebuffer::construct(device,
		{
			.extent = { 1024, 1024 },
			.attachments = { { ImageFormat::Depth, false } },
			.clear_colour_on_load = true,
			.clear_depth_on_load = true,
			.debug_name = "ShadowFramebuffer",
		});
	framebuffers.try_emplace(SceneFramebuffer::Shadow, std::move(temporary_shadow));

	const auto& shadow_framebuffer = get_framebuffer<SceneFramebuffer::Shadow>();
	get_graphics_resource().expose_to_shaders(shadow_framebuffer->get_depth_image(), DescriptorSet { 1 }, DescriptorBinding { 1 });
	SpecialisationConstantDescription specialisation_constant_description { point_light_data };
	resources.get_pipeline_cache().put(
		{
			.pipeline_key = "Shadow",
			.vertex_shader_key = "shadow.vert",
			.fragment_shader_key = "shadow.frag",
			.framebuffer = shadow_framebuffer,
			.layout = {
				{ ElementType::Float3, "position" },
				{ ElementType::Float2, "uv" },
				{ ElementType::Float4, "colour" },
				{ ElementType::Float3, "normals" },
				{ ElementType::Float3, "tangents" },
				{ ElementType::Float3, "bitangents" },
			},
			.push_constant_layout = { { PushConstantKind::Both, sizeof(PushConstant) } },
			.extent = renderer_extent,
			.cull_mode = CullMode::Front,
			.write_depth = true,
			.test_depth = true,
			.descriptor_set_layouts = desc_layout,
			.specialisation_constant = specialisation_constant_description,
		});

	resources.get_pipeline_cache().put(
		{
			.pipeline_key = "ShadowInstances",
			.vertex_shader_key = "shadow_instances.vert",
			.fragment_shader_key = "shadow.frag",
			.framebuffer = shadow_framebuffer,
			.layout = {
				{ ElementType::Float3, "position" },
				{ ElementType::Float2, "uv" },
				{ ElementType::Float4, "colour" },
				{ ElementType::Float3, "normals" },
				{ ElementType::Float3, "tangents" },
				{ ElementType::Float3, "bitangents" },
			},
			.push_constant_layout = { { PushConstantKind::Both, sizeof(PushConstant) } },
			.extent = renderer_extent,
			.cull_mode = CullMode::Front,
			.write_depth = true,
			.test_depth = true,
			.descriptor_set_layouts = desc_layout,
			.specialisation_constant = specialisation_constant_description,
		});

	framebuffers.try_emplace(SceneFramebuffer::Identity,
		Framebuffer::construct(device,
			{
				.extent = renderer_extent,
				.attachments = { { ImageFormat::Uint, false } },
				.clear_colour_on_load = true,
				.clear_depth_on_load = true,
				.should_blend = false,
				.blend_mode = FramebufferBlendMode::None,
				.debug_name = "IdentityFramebuffer",
			}));
	resources.get_pipeline_cache().put({
		.pipeline_key = "Identity",
		.vertex_shader_key = "identity.vert",
		.fragment_shader_key = "identity.frag",
		.framebuffer = get_framebuffer<SceneFramebuffer::Identity>(),
		.layout = { { ElementType::Float3, "position" }, { ElementType::Float2, "uvs" }, { ElementType::Float4, "colour" },
			{ ElementType::Float3, "normals" }, { ElementType::Float3, "tangents" }, { ElementType::Float3, "bitangents" } },
		.push_constant_layout = { { PushConstantKind::Both, sizeof(PushConstant) } },
		.extent = renderer_extent,
		.polygon_mode = PolygonMode::Fill,
		.cull_mode = CullMode::Back,
		.face_mode = FaceMode::Clockwise,
		.write_depth = true,
		.test_depth = true,
		.descriptor_set_layouts = resources.get_descriptor_set_layouts(),
		.specialisation_constant = specialisation_constant_description,

	});

	framebuffers.try_emplace(SceneFramebuffer::Geometry,
		Framebuffer::construct(device,
			{
				.extent = renderer_extent,
				.attachments = { { ImageFormat::SBGR }, { ImageFormat::Depth } },
				.clear_colour_on_load = false,
				.clear_depth_on_load = false,
				.should_blend = true,
				.blend_mode = FramebufferBlendMode::SrcAlphaOneMinusSrcAlpha,
				.debug_name = "GeometryFramebuffer",
			}));

	const auto& geometry_framebuffer = get_framebuffer<SceneFramebuffer::Geometry>();
	get_graphics_resource().expose_to_shaders(geometry_framebuffer->get_image(), DescriptorSet { 1 }, DescriptorBinding { 0 });

	resources.get_pipeline_cache().put({
		.pipeline_key = "PointLight",
		.vertex_shader_key = "point_light.vert",
		.fragment_shader_key = "point_light.frag",
		.framebuffer = get_framebuffer<SceneFramebuffer::Geometry>(),
		.layout = {
			{ ElementType::Float3, "position" },
			{ ElementType::Float2, "uv" },
			{ ElementType::Float4, "colour" },
			{ ElementType::Float3, "normals" },
			{ ElementType::Float3, "tangents" },
			{ ElementType::Float3, "bitangents" },
		},
		.push_constant_layout = { { PushConstantKind::Both, sizeof(PushConstant) } },
		.extent = renderer_extent,
		.cull_mode = CullMode::Front,
		.descriptor_set_layouts = desc_layout,
		.specialisation_constant = specialisation_constant_description,
	});

	resources.get_pipeline_cache().put(PipelineCacheCreationProperties {
		.pipeline_key = "Skybox",
		.vertex_shader_key = "skybox.vert",
		.fragment_shader_key = "skybox.frag",
		.framebuffer = get_framebuffer<SceneFramebuffer::Geometry>(),
		.layout = {
			{ ElementType::Float3, "position" },
			{ ElementType::Float2, "uv" },
			{ ElementType::Float4, "colour" },
			{ ElementType::Float3, "normals" },
			{ ElementType::Float3, "tangents" },
			{ ElementType::Float3, "bitangents" },
		},
		.push_constant_layout = { { PushConstantKind::Both, sizeof(PushConstant) } },
		.extent = renderer_extent,
		.cull_mode = CullMode::Back,
		.face_mode = FaceMode::CounterClockwise,
		.write_depth = false,
		.test_depth = false,
		.descriptor_set_layouts = desc_layout,
		.specialisation_constant = specialisation_constant_description,
	});

	resources.get_pipeline_cache().put({
		.pipeline_key = "StaticMesh",
		.vertex_shader_key = "static_mesh.vert",
		.fragment_shader_key = "static_mesh.frag",
		.framebuffer = get_framebuffer<SceneFramebuffer::Geometry>(),
		.layout = {
			{ ElementType::Float3, "position" },
			{ ElementType::Float2, "uv" },
			{ ElementType::Float4, "colour" },
			{ ElementType::Float3, "normals" },
			{ ElementType::Float3, "tangents" },
			{ ElementType::Float3, "bitangents" },
		},
		.push_constant_layout = { { PushConstantKind::Both, sizeof(PushConstant) } },
		.extent = renderer_extent,
		.cull_mode = CullMode::Front,
		.descriptor_set_layouts = desc_layout,
		.specialisation_constant = specialisation_constant_description,
	});
	point_light_data.calculate_point_lights = 1;
	resources.get_pipeline_cache().put({
		.pipeline_key = "StaticMeshNoPointLights",
		.vertex_shader_key = "static_mesh.vert",
		.fragment_shader_key = "static_mesh.frag",
		.framebuffer = get_framebuffer<SceneFramebuffer::Geometry>(),
		.layout = {
			{ ElementType::Float3, "position" },
			{ ElementType::Float2, "uv" },
			{ ElementType::Float4, "colour" },
			{ ElementType::Float3, "normals" },
			{ ElementType::Float3, "tangents" },
			{ ElementType::Float3, "bitangents" },
		},
		.push_constant_layout = { { PushConstantKind::Both, sizeof(PushConstant) } },
		.extent = renderer_extent,
		.cull_mode = CullMode::Front,
		.descriptor_set_layouts = desc_layout,
		.specialisation_constant = specialisation_constant_description,
	});

	point_light_transforms = StorageBuffer::construct_scoped(device,
		{
			.size = count_point_lights * sizeof(glm::mat4),
			.count = count_point_lights,
			.always_mapped = true,
		});
	point_light_colours = StorageBuffer::construct_scoped(device,
		{
			.size = count_point_lights * sizeof(glm::vec4),
			.count = count_point_lights,
			.always_mapped = true,
		});

	static constexpr auto max_identifier_objects = 2000;
	entity_identifiers = StorageBuffer::construct_scoped(device,
		{
			.size = max_identifier_objects * sizeof(std::uint32_t),
			.count = max_identifier_objects,
			.always_mapped = true,
		});
	entity_transforms = StorageBuffer::construct_scoped(device,
		{
			.size = max_identifier_objects * sizeof(glm::mat4),
			.count = max_identifier_objects,
			.always_mapped = true,
		});
	get_graphics_resource().expose_to_shaders(*point_light_transforms, DescriptorSet { 3 }, DescriptorBinding { 0 });
	get_graphics_resource().expose_to_shaders(*point_light_colours, DescriptorSet { 3 }, DescriptorBinding { 1 });
	get_graphics_resource().expose_to_shaders(*entity_identifiers, DescriptorSet { 3 }, DescriptorBinding { 2 });
	get_graphics_resource().expose_to_shaders(*entity_transforms, DescriptorSet { 3 }, DescriptorBinding { 3 });

	framebuffers.try_emplace(SceneFramebuffer::FullScreen,
		Framebuffer::construct(device,
			{
				.extent = renderer_extent,
				.attachments = { { ImageFormat::SBGR } },
				.debug_name = "FullScreenFramebuffer",
			}));
	get_pipeline_cache().put({
		.pipeline_key = "FullScreen",
		.vertex_shader_key = "fullscreen_quad.vert",
		.fragment_shader_key = "fullscreen_quad.frag",
		.framebuffer = get_framebuffer<SceneFramebuffer::FullScreen>(),
		.push_constant_layout = { { PushConstantKind::Both, sizeof(PushConstant) } },
		.extent = renderer_extent,
		.cull_mode = CullMode::Front,
		.write_depth = false,
		.test_depth = false,
		.descriptor_set_layouts = get_graphics_resource().get_descriptor_set_layouts(),
	});

	aabb_model = Mesh::construct_scoped(device,
		MeshProperties {
			.path = FS::model("cube.obj"),
		});
	get_pipeline_cache().put({
		.pipeline_key = "AABB",
		.vertex_shader_key = "static_mesh.vert",
		.fragment_shader_key = "static_mesh.frag",
		.framebuffer = geometry_framebuffer,
		.layout = { { ElementType::Float3, "position" }, { ElementType::Float2, "uvs" }, { ElementType::Float4, "colour" },
			{ ElementType::Float3, "normals" }, { ElementType::Float3, "tangents" }, { ElementType::Float3, "bitangents" } },
		.push_constant_layout = { { PushConstantKind::Both, sizeof(PushConstant) } },
		.extent = renderer_extent,
		.polygon_mode = PolygonMode::Line,
		.line_width = 3.0F,
		.cull_mode = CullMode::Front,
		.write_depth = true,
		.test_depth = true,
		.descriptor_set_layouts = get_graphics_resource().get_descriptor_set_layouts(),
	});

	PipelineCacheCreationProperties pipeline_properties = {
		.pipeline_key = "Quad",
		.vertex_shader_key = "quad.vert",
		.fragment_shader_key = "quad.frag",
		.framebuffer = geometry_framebuffer,
		.layout = { { ElementType::Float3, "position" }, { ElementType::Float2, "uvs" }, { ElementType::Float3, "normals" },
			{ ElementType::Float4, "colour" }, },
		.push_constant_layout = PushConstantLayout { PushConstantRange { PushConstantKind::Both, sizeof(PushConstant) } },
		.extent = renderer_extent,
		.cull_mode = CullMode::None,
		.descriptor_set_layouts = get_graphics_resource().get_descriptor_set_layouts(),
		.specialisation_constant = specialisation_constant_description,
	};

	get_pipeline_cache().put(pipeline_properties);
	// Line
	pipeline_properties.framebuffer = geometry_framebuffer;
	pipeline_properties.pipeline_key = "Line";
	pipeline_properties.vertex_shader_key = "line.vert";
	pipeline_properties.fragment_shader_key = "line.frag";
	pipeline_properties.write_depth = true;
	pipeline_properties.test_depth = true;
	pipeline_properties.line_width = 3.0F;
	pipeline_properties.polygon_mode = PolygonMode::Line;
	pipeline_properties.layout = { { ElementType::Float3, "pos" }, { ElementType::Float4, "colour" } };
	get_pipeline_cache().put(pipeline_properties);

	renderer->construct_sub_renderers(device, app);
}

auto SceneRenderer::interface() -> void
{
	UI::scope("Parameters", [&]() {
		bool any_changed_spec = false;
		any_changed_spec |= UI::Input::input("Point Light", &point_light_data.calculate_point_lights);
		any_changed_spec |= UI::Input::input("Gamma correct", &point_light_data.use_gamma_correction);

		if (UI::button("Clear Batch")) {
			command_executor->begin();
			clear_pass<RenderPasses::PlanarGeometry>();
			command_executor->submit_and_end();
		}
		if (UI::button("Clear Text")) {
			command_executor->begin();
			clear_pass<RenderPasses::Text>();
			command_executor->submit_and_end();
		}

		if (any_changed_spec) {
			get_pipeline_cache().force_recreation(renderer_extent);
		}
	});
}
auto SceneRenderer::recreate(bool should_clean, const Extent& extent) -> void
{
	renderer_extent = extent;

	framebuffers.at(SceneFramebuffer::Geometry)->recreate(should_clean, extent);
	framebuffers.at(SceneFramebuffer::Identity)->recreate(should_clean, extent);
	framebuffers.at(SceneFramebuffer::FullScreen)->recreate(should_clean, extent);

	const auto& old_extent = framebuffers.at(SceneFramebuffer::Shadow)->get_properties().extent;
	framebuffers.at(SceneFramebuffer::Shadow)->recreate(should_clean, old_extent);

	const auto& shadow_framebuffer = get_framebuffer<SceneFramebuffer::Shadow>();
	get_graphics_resource().expose_to_shaders(shadow_framebuffer->get_depth_image(), DescriptorSet { 1 }, DescriptorBinding { 1 });

	const auto& geometry_framebuffer = get_framebuffer<SceneFramebuffer::Geometry>();
	get_graphics_resource().expose_to_shaders(geometry_framebuffer->get_image(), DescriptorSet { 1 }, DescriptorBinding { 0 });

	get_graphics_resource().expose_to_shaders(*point_light_transforms, DescriptorSet { 3 }, DescriptorBinding { 0 });
	get_graphics_resource().expose_to_shaders(*point_light_colours, DescriptorSet { 3 }, DescriptorBinding { 1 });
	get_graphics_resource().expose_to_shaders(*entity_identifiers, DescriptorSet { 3 }, DescriptorBinding { 2 });
	get_graphics_resource().expose_to_shaders(*entity_transforms, DescriptorSet { 3 }, DescriptorBinding { 3 });

	command_executor->recreate(true, extent);
}

auto SceneRenderer::destruct() -> void
{
	for (auto&& [k, v] : framebuffers) {
		v.reset();
	}
	command_executor.reset();
}

auto SceneRenderer::get_pipeline(const std::string& key) -> Ref<Disarray::Pipeline>& { return get_pipeline_cache().get(key); }
auto SceneRenderer::get_graphics_resource() -> IGraphicsResource& { return renderer->get_graphics_resource(); }
auto SceneRenderer::get_texture_cache() -> TextureCache& { return renderer->get_texture_cache(); }
auto SceneRenderer::get_pipeline_cache() -> PipelineCache& { return renderer->get_pipeline_cache(); }
auto SceneRenderer::get_command_executor() -> CommandExecutor& { return *command_executor; }

auto SceneRenderer::fullscreen_quad_pass() -> void { renderer->fullscreen_quad_pass(*command_executor, *get_pipeline("FullScreen")); }

auto SceneRenderer::text_rendering_pass() -> void { renderer->text_rendering_pass(*command_executor); }

auto SceneRenderer::planar_geometry_pass() -> void { renderer->planar_geometry_pass(*command_executor); }

auto SceneRenderer::begin_frame(const glm::mat4& view, const glm::mat4& projection, const glm::mat4& view_projection) -> void
{
	renderer->begin_frame(view, projection, view_projection);
}

auto SceneRenderer::end_frame() -> void { renderer->end_frame(); }

void SceneRenderer::end_pass() { renderer->end_pass(*command_executor); }

auto SceneRenderer::draw_identifiers(std::size_t count) -> void
{
	const auto& vb = aabb_model->get_vertices();
	const auto& ib = aabb_model->get_indices();
	renderer->draw_mesh_instanced(*command_executor, count, vb, ib, *get_pipeline("Identity"));
}

auto SceneRenderer::draw_aabb(const Disarray::AABB& aabb, const glm::vec4& colour, const glm::mat4& transform) -> void
{
	const auto scale_matrix = aabb.calculate_scale_matrix();
	// Calculate the center of the AABB
	glm::vec3 aabb_center = aabb.middle_point();
	glm::vec3 translation = -aabb_center;

	glm::mat4 transformation_matrix = glm::mat4(1.0F);
	transformation_matrix = glm::translate(transformation_matrix, translation);
	transformation_matrix = transform * scale_matrix * transformation_matrix;
	draw_single_static_mesh(*aabb_model, *get_pipeline("AABB"), transformation_matrix, colour);
}

auto SceneRenderer::draw_text(const std::string& text_data, const glm::uvec2& position, float size, const glm::vec4& colour) -> void
{
	renderer->draw_text(text_data, position, size, colour);
}

auto SceneRenderer::draw_text(const std::string& text_data, const glm::mat4& transform, float size, const glm::vec4& colour) -> void
{
	renderer->draw_text(text_data, transform, size, colour);
}

auto SceneRenderer::draw_skybox(const Mesh& skybox_mesh) -> void
{
	draw_single_static_mesh(skybox_mesh, *get_pipeline("Skybox"), glm::mat4 {}, { 1, 1, 1, 1 });
}

auto SceneRenderer::draw_point_lights(const Mesh& point_light_mesh, std::uint32_t count, const Pipeline& pipeline) -> void
{
	renderer->draw_mesh_instanced(*command_executor, count, point_light_mesh.get_vertices(), point_light_mesh.get_indices(), pipeline);
}

auto SceneRenderer::draw_planar_geometry(Geometry geometry, const GeometryProperties& properties) -> void
{
	renderer->draw_planar_geometry(geometry, properties);
}

auto SceneRenderer::draw_submesh(Disarray::CommandExecutor&, const Disarray::VertexBuffer& vertex_buffer, const Disarray::IndexBuffer& index_buffer,
	const Disarray::Pipeline& pipeline, const glm::vec4& colour, const glm::mat4& transform, Disarray::PushConstant& push_constant) -> void
{
	push_constant.object_transform = transform;
	push_constant.colour = colour;
	draw_single_static_mesh(vertex_buffer, index_buffer, pipeline, transform, colour);
}

auto SceneRenderer::draw_static_submeshes(const Collections::ScopedStringMap<MeshSubstructure>& submeshes, const Pipeline& pipeline,
	const glm::mat4& transform, const glm::vec4& colour) -> void
{
	renderer->bind_descriptor_sets(*command_executor, pipeline);
	renderer->bind_pipeline(*command_executor, pipeline);

	auto& push_constant = get_graphics_resource().get_editable_push_constant();

	Collections::for_each(submeshes, [&](const auto& sub) {
		auto&& [key, mesh] = sub;

		std::size_t index = 0;
		for (const auto& texture_index : mesh->texture_indices) {
			push_constant.image_indices.at(index++) = static_cast<int>(texture_index);
		}
		push_constant.bound_textures = static_cast<unsigned int>(index);
		draw_submesh(*command_executor, *mesh->vertices, *mesh->indices, pipeline, colour, transform, push_constant);
		push_constant.bound_textures = 0;
	});
}

auto SceneRenderer::draw_single_static_mesh(const Mesh& mesh, const Pipeline& pipeline, const glm::mat4& transform, const glm::vec4& colour) -> void
{
	renderer->draw_mesh(*command_executor, mesh, pipeline, colour, transform);
}

auto SceneRenderer::draw_single_static_mesh(
	const VertexBuffer& vertices, const IndexBuffer& indices, const Pipeline& pipeline, const glm::mat4& transform, const glm::vec4& colour) -> void
{
	renderer->draw_mesh(*command_executor, vertices, indices, pipeline, colour, transform);
}

auto SceneRenderer::begin_pass(const Disarray::Framebuffer& framebuffer, bool explicit_clear) -> void
{
	renderer->begin_pass(*command_executor, const_cast<Disarray::Framebuffer&>(framebuffer), explicit_clear);
}

void SceneRenderer::clear_pass(RenderPasses render_pass) { renderer->clear_pass(*command_executor, render_pass); }

auto SceneRenderer::get_final_image() -> const Disarray::Image& { return framebuffers.at(SceneFramebuffer::FullScreen)->get_image(); }

void SceneRenderer::begin_execution() { command_executor->begin(); }

void SceneRenderer::submit_executed_commands() { command_executor->submit_and_end(); }

} // namespace Disarray