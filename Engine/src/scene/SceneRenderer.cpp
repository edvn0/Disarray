#include "DisarrayPCH.hpp"

#include <magic_enum_switch.hpp>

#include "core/App.hpp"
#include "core/filesystem/AssetLocations.hpp"
#include "graphics/BufferProperties.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/GLM.hpp"
#include "graphics/Maths.hpp"
#include "graphics/Mesh.hpp"
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
#include "graphics/UniformBufferSet.hpp"
#include "scene/Components.hpp"
#include "scene/SceneRenderer.hpp"
#include "ui/UI.hpp"

namespace Disarray {

auto get_current_frame_index(const IGraphicsResource& graphics_resource) -> FrameIndex { return graphics_resource.get_current_frame_index(); }
auto get_image_count(const IGraphicsResource& graphics_resource) -> std::uint32_t { return graphics_resource.get_image_count(); }

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

	const VertexLayout default_inputs {
		{ ElementType::Float3, "position" },
		{ ElementType::Float2, "uv" },
		{ ElementType::Float4, "colour" },
		{ ElementType::Float3, "normals" },
		{ ElementType::Float3, "tangents" },
		{ ElementType::Float3, "bitangents" },
	};

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
	get_graphics_resource().expose_to_shaders(shadow_framebuffer->get_depth_image(), DescriptorSet { 0 }, DescriptorBinding { 10 });

	const SpecialisationConstantDescription specialisation_constant_description { point_light_data };
	resources.get_pipeline_cache().put({
		.pipeline_key = "Shadow",
		.vertex_shader_key = "shadow.vert",
		.fragment_shader_key = "shadow.frag",
		.framebuffer = shadow_framebuffer,
		.layout = {},
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
		.layout = default_inputs,
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
				.attachments = { { ImageFormat::SRGB32 }, { ImageFormat::Depth } },
				.clear_colour_on_load = false,
				.clear_depth_on_load = false,
				.should_blend = true,
				.blend_mode = FramebufferBlendMode::SrcAlphaOneMinusSrcAlpha,
				.debug_name = "GeometryFramebuffer",
			}));

	const auto& geometry_framebuffer = get_framebuffer<SceneFramebuffer::Geometry>();
	get_graphics_resource().expose_to_shaders(geometry_framebuffer->get_image(), DescriptorSet { 0 }, DescriptorBinding { 11 });

	resources.get_pipeline_cache().put(PipelineCacheCreationProperties {
		.pipeline_key = "Skybox",
		.vertex_shader_key = "skybox.vert",
		.fragment_shader_key = "skybox.frag",
		.framebuffer = get_framebuffer<SceneFramebuffer::Geometry>(),
		.layout = default_inputs,
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
		.layout = default_inputs,
		.push_constant_layout = { { PushConstantKind::Vertex, sizeof(PushConstant) }, { PushConstantKind::Fragment, sizeof(PushConstant) } },
		.extent = renderer_extent,
		.cull_mode = CullMode::Front,
		.descriptor_set_layouts = desc_layout,
		.specialisation_constant = specialisation_constant_description,
	});

	auto static_mesh_combined = UnifiedShader::construct(device,
		{
			.path = FS::shader("static_mesh_combined.glsl"),
		});
	geometry_pipeline = Pipeline::construct(device,
		{
			.combined_shader = static_mesh_combined,
			.framebuffer = get_framebuffer<SceneFramebuffer::Geometry>(),
			.layout = default_inputs,
			.push_constant_layout = { { PushConstantKind::Vertex, sizeof(PushConstant) }, { PushConstantKind::Fragment, sizeof(PushConstant) } },
			.extent = renderer_extent,
			.cull_mode = CullMode::Front,
			.descriptor_set_layouts = desc_layout,
			.specialisation_constants = specialisation_constant_description,
		});

	/*{
		.pipeline_key = "StaticMeshCombined",
		.combined_key = "static_mesh_combined.glsl",
		.framebuffer = get_framebuffer<SceneFramebuffer::Geometry>(),
		.layout = default_inputs,
		.push_constant_layout = { { PushConstantKind::Vertex, sizeof(PushConstant) }, { PushConstantKind::Fragment, sizeof(PushConstant) } },
		.extent = renderer_extent,
		.cull_mode = CullMode::Front,
		.descriptor_set_layouts = desc_layout,
		.specialisation_constant = specialisation_constant_description,
	});*/

	point_light_data.calculate_point_lights = 1;
	resources.get_pipeline_cache().put({
		.pipeline_key = "StaticMeshNoPointLights",
		.vertex_shader_key = "static_mesh.vert",
		.fragment_shader_key = "static_mesh.frag",
		.framebuffer = get_framebuffer<SceneFramebuffer::Geometry>(),
		.layout = default_inputs,
		.push_constant_layout = { { PushConstantKind::Both, sizeof(PushConstant) } },
		.extent = renderer_extent,
		.cull_mode = CullMode::Front,
		.descriptor_set_layouts = desc_layout,
		.specialisation_constant = specialisation_constant_description,
	});
	point_light_data.calculate_point_lights = 0;

	uniform_buffer_set = make_scope<BufferSet<UniformBuffer>>(device, get_graphics_resource(), 1);
	uniform_buffer_set->add_buffer<ViewProjectionUBO>(DescriptorSet { 0 }, DescriptorBinding { 0 });
	uniform_buffer_set->add_buffer<CameraUBO>(DescriptorSet { 0 }, DescriptorBinding { 1 });
	uniform_buffer_set->add_buffer<PointLight>(DescriptorSet { 0 }, DescriptorBinding { 2 });
	uniform_buffer_set->add_buffer<ShadowPassUBO>(DescriptorSet { 0 }, DescriptorBinding { 3 });
	uniform_buffer_set->add_buffer<DirectionalLightUBO>(DescriptorSet { 0 }, DescriptorBinding { 4 });
	uniform_buffer_set->add_buffer<GlyphUBO>(DescriptorSet { 0 }, DescriptorBinding { 5 });
	uniform_buffer_set->add_buffer<SpotLights>(DescriptorSet { 0 }, DescriptorBinding { 6 });

	storage_buffer_set = make_scope<BufferSet<StorageBuffer>>(device, get_graphics_resource(), 1);
	static constexpr auto max_identifier_objects = 2000;
	storage_buffer_set->add_buffer(max_identifier_objects * sizeof(glm::mat4), max_identifier_objects, DescriptorSet { 0 }, DescriptorBinding { 7 });
	storage_buffer_set->add_buffer(
		max_identifier_objects * sizeof(std::uint32_t), max_identifier_objects, DescriptorSet { 0 }, DescriptorBinding { 8 });

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
		.layout = default_inputs,
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
		.layout = {
			{ ElementType::Float3, "position" },
			{ ElementType::Float2, "uvs" },
			{ ElementType::Float3, "normals" },
			{ ElementType::Float4, "colour" },
		},
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
	pipeline_properties.layout = {
		{ ElementType::Float3, "pos" },
		{ ElementType::Float4, "colour" },
	};
	get_pipeline_cache().put(pipeline_properties);

	DataBuffer white_data_buffer;
	white_data_buffer.allocate(sizeof(std::uint32_t));
	white_data_buffer.write(0xFFFFFFFF);

	const auto& white_texture = get_texture_cache().put({
		.key = "White",
		.debug_name = "White",
		.data_buffer = white_data_buffer,
		.extent = { 1, 1 },
	});
	default_material = Material::construct_scoped(
		device,
		{
			.textures = {
				{ "albedo", white_texture },
				{ "diffuse", white_texture },
				{ "specular", white_texture },
			},
		});
	default_material->write_textures(get_graphics_resource());

	renderer->construct_sub_renderers(device, app);
}

auto SceneRenderer::interface() -> void
{
	UI::scope("Parameters", [&]() {
		bool any_changed_spec = false;
		any_changed_spec |= UI::Input::input("Point Light", &point_light_data.calculate_point_lights);
		any_changed_spec |= UI::Input::input("Gamma correct", &point_light_data.use_gamma_correction);

		if (UI::button("Clear Batch", { 80, 30 })) {
			command_executor->begin();
			clear_pass<RenderPasses::PlanarGeometry>();
			command_executor->submit_and_end();
		}
		ImGui::SameLine();
		if (UI::button("Clear Text", { 80, 30 })) {
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
	get_graphics_resource().expose_to_shaders(shadow_framebuffer->get_depth_image(), DescriptorSet { 0 }, DescriptorBinding { 10 });

	const auto& geometry_framebuffer = get_framebuffer<SceneFramebuffer::Geometry>();
	get_graphics_resource().expose_to_shaders(geometry_framebuffer->get_image(), DescriptorSet { 0 }, DescriptorBinding { 11 });

	default_material->write_textures(get_graphics_resource());
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

auto SceneRenderer::text_rendering_pass() -> void
{
	{
		auto& glyph = uniform_buffer_set->for_frame(DescriptorSet(0), DescriptorBinding(6));
		auto& view_projection = uniform_buffer_set->for_frame(DescriptorSet(0), DescriptorBinding(0));
		const auto float_extent = renderer_extent.as<float>();
		GlyphUBO buffer {};
		buffer.projection = Maths::ortho(0.F, float_extent.width, 0.F, float_extent.height, -1.0F, 1.0F);
		const ViewProjectionUBO& ubo = *static_cast<const ViewProjectionUBO*>(view_projection->get_raw());
		buffer.view = ubo.view;

		glyph->set_data<GlyphUBO>(&buffer);
	}

	renderer->text_rendering_pass(*command_executor);
}

auto SceneRenderer::planar_geometry_pass() -> void { renderer->planar_geometry_pass(*command_executor); }

auto SceneRenderer::begin_frame(const TransformMatrix& view, const TransformMatrix& projection, const TransformMatrix& view_projection) -> void
{
	CameraUBO camera {};
	ViewProjectionUBO default_ubo {};

	const auto view_matrix = glm::inverse(view);
	camera.position = view_matrix[3];
	camera.direction = -view_matrix[2];
	default_ubo.view = view;
	default_ubo.proj = projection;
	default_ubo.view_projection = view_projection;

	camera.view = view;

	const auto& camera_buffer = uniform_buffer_set->for_frame(DescriptorSet(0), DescriptorBinding(3));
	camera_buffer->set_data<CameraUBO>(&camera);
	const auto& default_ubo_buffer = uniform_buffer_set->for_frame(DescriptorSet(0), DescriptorBinding(3));
	default_ubo_buffer->set_data<ViewProjectionUBO>(&default_ubo);

	renderer->begin_frame();
}

auto SceneRenderer::end_frame() -> void { renderer->end_frame(); }

void SceneRenderer::end_pass() { renderer->end_pass(*command_executor); }

auto SceneRenderer::draw_identifiers(std::size_t count) -> void
{
	const auto& vb = aabb_model->get_vertices();
	const auto& ib = aabb_model->get_indices();
	renderer->bind_buffer_set(*uniform_buffer_set, *storage_buffer_set);
	renderer->draw_mesh_instanced(*command_executor, count, vb, ib, *get_pipeline("Identity"));
}

auto SceneRenderer::draw_aabb(const Disarray::AABB&, const ColourVector& colour, const TransformMatrix& transform) -> void
{
	renderer->bind_buffer_set(*uniform_buffer_set, *storage_buffer_set);

	draw_single_static_mesh(*aabb_model, *get_pipeline("AABB"), transform, colour);
}

auto SceneRenderer::draw_text(const Components::Transform& transform, const Components::Text& text, const ColourVector& colour) -> void
{
	renderer->bind_buffer_set(*uniform_buffer_set, *storage_buffer_set);

	switch (text.projection) {
	case Components::TextProjection::Billboard: {
		renderer->draw_billboarded_text(text.text_data, transform.compute(), text.size, colour);
		break;
	}
	case Components::TextProjection::ScreenSpace: {
		renderer->draw_text(text.text_data, glm::uvec2(transform.position), text.size, colour);
		break;
	}
	case Components::TextProjection::WorldSpace: {
		renderer->draw_text(text.text_data, transform.position, text.size, colour);
		break;
	}
	};
}

auto SceneRenderer::draw_skybox() -> void
{
	renderer->bind_buffer_set(*uniform_buffer_set, *storage_buffer_set);
	draw_single_static_mesh(*aabb_model, *get_pipeline("Skybox"), glm::mat4 {}, { 1, 1, 1, 1 });
}

auto SceneRenderer::draw_point_lights(const Mesh& point_light_mesh, std::uint32_t count, const Pipeline& pipeline) -> void
{
	renderer->bind_buffer_set(*uniform_buffer_set, *storage_buffer_set);

	renderer->draw_mesh_instanced(*command_executor, count, point_light_mesh.get_vertices(), point_light_mesh.get_indices(), pipeline);
}

auto SceneRenderer::draw_planar_geometry(Geometry geometry, const GeometryProperties& properties) -> void
{
	renderer->draw_planar_geometry(geometry, properties);
}

auto SceneRenderer::draw_static_submeshes(const Collections::ScopedStringMap<Disarray::Mesh>& submeshes, const Pipeline& pipeline,
	const TransformMatrix& transform, const ColourVector& colour) -> void
{
	renderer->bind_buffer_set(*uniform_buffer_set, *storage_buffer_set);

	renderer->bind_descriptor_sets(*command_executor, pipeline);
	renderer->bind_pipeline(*command_executor, pipeline);
	auto& editable = renderer->get_graphics_resource().get_editable_push_constant();
	editable.object_transform = transform;
	editable.albedo_colour = colour;
	renderer->push_constant(*command_executor, pipeline);

	Collections::for_each_unwrapped(
		submeshes, [this](const auto&, const Scope<Disarray::Mesh>& mesh) { renderer->draw_mesh_without_bind(*command_executor, *mesh); });
}

auto SceneRenderer::draw_single_static_mesh(const Disarray::Mesh& mesh, const Disarray::Pipeline& pipeline, const Disarray::Material& material,
	const TransformMatrix& transform, const ColourVector& colour) -> void
{
	renderer->bind_buffer_set(*uniform_buffer_set, *storage_buffer_set);

	renderer->draw_mesh(*command_executor, mesh, pipeline, material, transform, colour);
}

auto SceneRenderer::draw_single_static_mesh(const Disarray::VertexBuffer& vertices, const Disarray::IndexBuffer& indices,
	const Disarray::Pipeline& pipeline, const Disarray::Material& material, const TransformMatrix& transform, const ColourVector& colour) -> void
{
	renderer->bind_buffer_set(*uniform_buffer_set, *storage_buffer_set);

	renderer->draw_mesh(*command_executor, vertices, indices, pipeline, material, transform, colour);
}

auto SceneRenderer::draw_single_static_mesh(const Mesh& mesh, const Pipeline& pipeline, const TransformMatrix& transform, const ColourVector& colour)
	-> void
{
	renderer->bind_buffer_set(*uniform_buffer_set, *storage_buffer_set);

	renderer->draw_mesh(*command_executor, mesh, pipeline, *default_material, transform, colour);
}

auto SceneRenderer::draw_single_static_mesh(const VertexBuffer& vertices, const IndexBuffer& indices, const Pipeline& pipeline,
	const TransformMatrix& transform, const ColourVector& colour) -> void
{
	renderer->bind_buffer_set(*uniform_buffer_set, *storage_buffer_set);

	renderer->draw_mesh(*command_executor, vertices, indices, pipeline, *default_material, transform, colour);
}

auto SceneRenderer::begin_pass(const Disarray::Framebuffer& framebuffer, bool explicit_clear) -> void
{
	renderer->begin_pass(*command_executor, const_cast<Disarray::Framebuffer&>(framebuffer), explicit_clear);
}

void SceneRenderer::clear_pass(RenderPasses render_pass) { renderer->clear_pass(*command_executor, render_pass); }

auto SceneRenderer::get_final_image() -> const Disarray::Image& { return framebuffers.at(SceneFramebuffer::FullScreen)->get_image(); }

void SceneRenderer::begin_execution() { command_executor->begin(); }

void SceneRenderer::submit_executed_commands() { command_executor->submit_and_end(); }

void SceneRenderer::draw_static_mesh(const Disarray::StaticMesh& mesh, const Disarray::MaterialTable& table, const TransformMatrix& transform)
{
	const auto& vertex_buffer = mesh.get_vertices();
	const auto& index_buffer = mesh.get_indices();
	renderer->bind_buffer(*command_executor, index_buffer);
	renderer->bind_buffer(*command_executor, vertex_buffer);

	for (const auto& submesh : mesh.get_submeshes()) {
		renderer->draw_static_mesh(*command_executor, *geometry_pipeline, *uniform_buffer_set, *storage_buffer_set, submesh, mesh, table, transform);
	}
}

} // namespace Disarray
