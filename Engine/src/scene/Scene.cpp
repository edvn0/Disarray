#include "DisarrayPCH.hpp"

#include "scene/Scene.hpp"

#include <ImGuizmo.h>
#include <entt/entt.hpp>

#include <array>
#include <mutex>
#include <string_view>
#include <thread>

#include "core/App.hpp"
#include "core/Collections.hpp"
#include "core/Ensure.hpp"
#include "core/Formatters.hpp"
#include "core/Input.hpp"
#include "core/Instrumentation.hpp"
#include "core/Random.hpp"
#include "core/ThreadPool.hpp"
#include "core/events/Event.hpp"
#include "core/events/KeyEvent.hpp"
#include "core/events/MouseEvent.hpp"
#include "core/filesystem/AssetLocations.hpp"
#include "entt/entity/fwd.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/GLM.hpp"
#include "graphics/Maths.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/PushConstantLayout.hpp"
#include "graphics/RenderBatch.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/RendererProperties.hpp"
#include "graphics/ShaderCompiler.hpp"
#include "graphics/StorageBuffer.hpp"
#include "graphics/Swapchain.hpp"
#include "graphics/Texture.hpp"
#include "graphics/TextureCache.hpp"
#include "scene/Camera.hpp"
#include "scene/Components.hpp"
#include "scene/CppScript.hpp"
#include "scene/Deserialiser.hpp"
#include "scene/Entity.hpp"
#include "scene/Scripts.hpp"
#include "scene/Serialiser.hpp"
#include "ui/UI.hpp"

namespace Disarray {

Scene::Scene(const Device& dev, std::string_view name)
	: device(dev)
	, scene_name(name)
{
	picked_entity = make_scope<Entity>(this);
	selected_entity = make_scope<Entity>(this);
}

void Scene::setup_filewatcher_and_threadpool(Threading::ThreadPool& pool)
{
	static constexpr auto tick_time = std::chrono::milliseconds(3000);
	file_watcher = make_scope<FileWatcher>(pool, "Assets/Shaders", Collections::StringSet { ".vert", ".frag", ".glsl" }, tick_time);
	auto& pipeline_cache = scene_renderer->get_graphics_resource().get_pipeline_cache();
	file_watcher->on_modified(
		[&renderer = scene_renderer, &pipeline_cache, &dev = device, &reg = registry, &mutex = registry_access](const FileInformation& entry) {
			std::scoped_lock lock { mutex };
			Log::info("Scene FileWatcher", "Recompiling entry: {}", entry.to_path());
			const auto view = reg.view<Components::Pipeline>();
			const auto as_path = entry.to_path();

			if (as_path.extension() == ".glsl") {
				auto glsl_shader = Shader::compile(dev, entry.path);
				pipeline_cache.update(as_path, std::move(glsl_shader));
			}

			std::unordered_set<Disarray::Pipeline*> pipelines_sharing_changed_shader {};
			for (auto&& [ent, pipeline] : view.each()) {
				const bool pipeline_uses_this_updated_shader = pipeline.pipeline->has_shader_with_name(as_path);
				if (!pipeline_uses_this_updated_shader) {
					continue;
				}

				pipelines_sharing_changed_shader.insert(pipeline.pipeline.get());
			}

			for (auto* pipeline : renderer->get_text_renderer().get_pipelines()) {
				const bool pipeline_uses_this_updated_shader = pipeline->has_shader_with_name(as_path);
				if (!pipeline_uses_this_updated_shader) {
					continue;
				}

				pipelines_sharing_changed_shader.insert(pipeline);
			}

			for (auto* pipeline : renderer->get_batch_renderer().get_pipelines()) {
				const bool pipeline_uses_this_updated_shader = pipeline->has_shader_with_name(as_path);
				if (!pipeline_uses_this_updated_shader) {
					continue;
				}

				pipelines_sharing_changed_shader.insert(pipeline);
			}

			if (pipelines_sharing_changed_shader.empty()) {
				return;
			}

			Ref<Shader> shader = nullptr;
			try {
				shader = Shader::compile(dev, entry.path);
			} catch (const CouldNotCompileShaderException& exc) {
				Log::info("Scene FileWatcher", "{}", exc);
				return;
			}

			for (auto* pipe : pipelines_sharing_changed_shader) {
				pipe->get_properties().set_shader_with_type(shader->get_properties().type, shader);
			}

			wait_for_idle(dev);
			for (auto* pipe : pipelines_sharing_changed_shader) {
				pipe->force_recreation();
			}
			wait_for_idle(dev);
			Log::info("Scene FileWatcher", "Finished compilation and {} pipelines reconstructed.", pipelines_sharing_changed_shader.size());
		});
}

void Scene::construct(Disarray::App& app)
{
	scene_renderer = Renderer::construct_unique(device, app.get_swapchain(), {});

	auto& pool = App::get_thread_pool();
	setup_filewatcher_and_threadpool(pool);

	extent = app.get_swapchain().get_extent();
	command_executor = CommandExecutor::construct(device, &app.get_swapchain(), { .count = 3, .is_primary = true, .record_stats = true });
	scene_renderer->on_batch_full([&exec = *command_executor](Renderer& renderer) { renderer.flush_batch(exec); });

	const auto& resources = scene_renderer->get_graphics_resource();
	const auto& desc_layout = resources.get_descriptor_set_layouts();

	{
		auto temporary_shadow = Framebuffer::construct(device,
			{
				.extent = { 4096, 4096 },
				.attachments = { { ImageFormat::Depth, false } },
				.clear_colour_on_load = true,
				.clear_depth_on_load = true,
				.debug_name = "ShadowFramebuffer",
			});
		framebuffers.try_emplace(SceneFramebuffer::Shadow, std::move(temporary_shadow));

		const auto& shadow_framebuffer = get_framebuffer<SceneFramebuffer::Shadow>();
		scene_renderer->get_graphics_resource().expose_to_shaders(shadow_framebuffer->get_depth_image(), 1, 1);

		shadow_pipeline = Pipeline::construct(device,
		{
			.vertex_shader = scene_renderer->get_pipeline_cache().get_shader("shadow.vert"),
			.fragment_shader = scene_renderer->get_pipeline_cache().get_shader("shadow.frag"),
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
			.cull_mode = CullMode::Front,
			.write_depth = true,
			.test_depth = true,
			.descriptor_set_layouts = desc_layout,
		});

		shadow_instances_pipeline = Pipeline::construct(device,
		{
			.vertex_shader = scene_renderer->get_pipeline_cache().get_shader("shadow_instances.vert"),
			.fragment_shader = scene_renderer->get_pipeline_cache().get_shader("shadow.frag"),
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
			.cull_mode = CullMode::Front,
			.write_depth = true,
			.test_depth = true,
			.descriptor_set_layouts = desc_layout,
		});
	}

	{
		framebuffers.try_emplace(SceneFramebuffer::Identity,
			Framebuffer::construct(device,
				{
					.extent = extent,
					.attachments = { { ImageFormat::Uint, false } },
					.clear_colour_on_load = true,
					.clear_depth_on_load = true,
					.should_blend = false,
					.blend_mode = FramebufferBlendMode::None,
					.debug_name = "IdentityFramebuffer",
				}));
		identity_pipeline = Pipeline::construct(device,
			PipelineProperties {
				.vertex_shader = scene_renderer->get_pipeline_cache().get_shader("identity.vert"),
				.fragment_shader = scene_renderer->get_pipeline_cache().get_shader("identity.frag"),
				.framebuffer = get_framebuffer<SceneFramebuffer::Identity>(),
				.layout = { { ElementType::Float3, "position" }, { ElementType::Float2, "uvs" }, { ElementType::Float4, "colour" },
					{ ElementType::Float3, "normals" }, { ElementType::Float3, "tangents" }, { ElementType::Float3, "bitangents" } },
				.push_constant_layout = { { PushConstantKind::Both, sizeof(PushConstant) } },
				.extent = extent,
				.polygon_mode = PolygonMode::Fill,
				.cull_mode = CullMode::Back,
				.face_mode = FaceMode::Clockwise,
				.write_depth = true,
				.test_depth = true,
				.descriptor_set_layouts = resources.get_descriptor_set_layouts(),
			});
	}

	{
		framebuffers.try_emplace(SceneFramebuffer::Geometry,
			Framebuffer::construct(device,
				{
					.extent = extent,
					.attachments = { { ImageFormat::SBGR }, { ImageFormat::Depth } },
					.clear_colour_on_load = true,
					.clear_depth_on_load = true,
					.should_blend = true,
					.blend_mode = FramebufferBlendMode::SrcAlphaOneMinusSrcAlpha,
					.debug_name = "GeometryFramebuffer",
				}));

		const auto& geometry_framebuffer = get_framebuffer<SceneFramebuffer::Geometry>();
		scene_renderer->get_graphics_resource().expose_to_shaders(geometry_framebuffer->get_image(), 1, 0);
	}

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
	scene_renderer->get_graphics_resource().expose_to_shaders(*point_light_transforms, 3, 0);
	scene_renderer->get_graphics_resource().expose_to_shaders(*point_light_colours, 3, 1);
}

Scene::~Scene() = default;

void Scene::begin_frame(const Camera& camera)
{
	auto find_one = get_by_components<Components::Camera>();
	bool camera_component_usable = true;
	if (find_one) {
		auto&& [camera_component, transform] = find_one->get_components<Components::Camera, const Components::Transform>();
		// if we have a primary camera, then camera_component_usable should be false
		if (camera_component.is_primary) {
			auto&& [projection, view, pre_mul] = camera_component.compute(transform, extent);
			scene_renderer->begin_frame(view, projection, pre_mul);
		} else {
			camera_component_usable = false;
		}
	}

	if (!camera_component_usable) {
		scene_renderer->begin_frame(camera);
	}

	auto [ubo, camera_ubo, light_ubos, shadow_pass, directional, glyph] = scene_renderer->get_graphics_resource().get_editable_ubos();
	auto& push_constant = scene_renderer->get_graphics_resource().get_editable_push_constant();

	for (auto sun_component_view = registry.view<const Components::Transform, Components::DirectionalLight>();
		 auto&& [entity, transform, sun] : sun_component_view.each()) {
		directional.position = { transform.position, 1.0F };
		sun.position = directional.position;
		sun.direction = glm::normalize(-sun.position); // Lookat {0,0,0};
		directional.direction = sun.direction;
		directional.ambient = sun.ambient;
		directional.diffuse = sun.diffuse;
		directional.specular = sun.specular;
		directional.near_far = glm::vec4 { 0 };
	}

	auto maybe_directional = get_by_components<Components::DirectionalLight, Components::Transform>();
	if (maybe_directional.has_value()) {

		auto&& [transform, light] = maybe_directional->get_components<Components::Transform, Components::DirectionalLight>();
		auto projection = light.projection_parameters.compute();
		glm::vec3 center { 0 };
		if (light.use_direction_vector) {
			auto lookat_center = transform.position + glm::vec3(light.direction);
			center = lookat_center;
			projection = glm::perspective(
				light.projection_parameters.fov, extent.aspect_ratio(), light.projection_parameters.near, light.projection_parameters.far);
			directional.near_far = { light.projection_parameters.near, light.projection_parameters.far, 0, 0 };
		}
		const auto view = glm::lookAt(glm::vec3(light.position), center, { 0.0F, 1.0F, 0.0F });

		const auto view_projection = projection * view;

		shadow_pass.view = view;
		shadow_pass.projection = projection;
		shadow_pass.view_projection = view_projection;
	}

	std::size_t light_index { 0 };
	auto& lights = light_ubos.lights;
	auto point_light_ssbo = point_light_transforms->get_mutable<glm::mat4>();
	auto point_light_ssbo_colour = point_light_colours->get_mutable<glm::vec4>();
	for (auto&& [entity, point_light, pos, texture] :
		registry.view<const Components::PointLight, const Components::Transform, Components::Texture>().each()) {
		auto& light = lights.at(light_index);
		light.position = glm::vec4 { pos.position, 0.F };
		light.ambient = point_light.ambient;
		light.diffuse = point_light.diffuse;
		light.specular = point_light.specular;
		light.factors = point_light.factors;
		texture.colour = light.ambient;

		point_light_ssbo[light_index] = pos.compute();
		point_light_ssbo_colour[light_index] = texture.colour;
		light_index++;
	}
	push_constant.max_point_lights = static_cast<std::uint32_t>(light_index);

	scene_renderer->get_graphics_resource().update_ubo();
}

void Scene::end_frame() { scene_renderer->end_frame(); }

void Scene::interface()
{
	auto script_view = registry.view<Components::Script>();
	for (auto&& [entity, script] : script_view.each()) {
		script.get_script().on_interface();
	}
}

void Scene::update(float time_step)
{
	if (picked_entity) {
		selected_entity.swap(picked_entity);
		picked_entity = nullptr;
	}

	auto pipeline_view = registry.view<Components::Pipeline>();
	for (const auto& [entity, pipeline] : pipeline_view.each()) {
		if (pipeline.pipeline->is_valid()) {
			continue;
		}

		auto& props = pipeline.pipeline->get_properties();
		props.descriptor_set_layouts = scene_renderer->get_graphics_resource().get_descriptor_set_layouts();
		switch (props.fragment_shader->attachment_count()) {
		case 1:
			props.framebuffer = get_framebuffer<SceneFramebuffer::Geometry>();
			break;
		default: {
			throw UnreachableException("Should never happen!");
		}
		}

		pipeline.pipeline->force_recreation();
	}
	{
		auto script_view = registry.view<Components::Script>();
		script_view.each([scene = this, step = time_step](const auto entity, Components::Script& script) {
			if (script.has_been_bound()) {
				script.instantiate();
			}

			auto& instantiated = script.get_script();
			instantiated.update_entity(scene, entity);
			instantiated.on_update(step);
		});
	}
	{
		auto controller_view = registry.view<Components::Controller, Components::Transform>();
		controller_view.each([step = time_step](const auto entity, Components::Controller& controller, Components::Transform& transform) {
			controller.on_update(step, transform);
		});
	}
}

void Scene::render()
{
	command_executor->begin();
	auto& executor = *command_executor;
	{
		// Shadow pass
		scene_renderer->begin_pass(executor, *get_framebuffer<SceneFramebuffer::Shadow>());
		draw_shadows();
		scene_renderer->planar_geometry_pass(executor);
		scene_renderer->end_pass(executor);
	}
	{
		// Geometry pass
		scene_renderer->begin_pass(executor, *get_framebuffer<SceneFramebuffer::Geometry>());
		draw_geometry();
		scene_renderer->planar_geometry_pass(executor);
		scene_renderer->end_pass(executor);
	}
#ifdef DISARRAY_DRAW_IDENTIFIERS
	{
		// Identifier pass
		scene_renderer->begin_pass(executor, *get_framebuffer<SceneFramebuffer::Identity>());
		draw_identifiers();
		scene_renderer->end_pass(executor);
	}
#endif
	{
		scene_renderer->draw_text({ 0, 0 }, "Hello world! {}\nHello", "Edwin");
		scene_renderer->draw_text({ 10, -5, 20 }, "Hello world! {}\nHello", "Edwin");
		scene_renderer->text_rendering_pass(executor);
	}
	{
		// This is the composite pass!
		scene_renderer->fullscreen_quad_pass(executor, extent);
	}

	command_executor->submit_and_end();
}

void Scene::draw_identifiers()
{
	scene_renderer->bind_pipeline(*command_executor, *identity_pipeline);
	scene_renderer->bind_descriptor_sets(*command_executor, *identity_pipeline);
	for (auto&& [entity, transform, tag] : registry.view<const Components::Transform, const Components::Tag>().each()) {
		if (tag.name == "Floor") [[unlikely]] {
			continue;
		}
		scene_renderer->draw_identifier(*command_executor, *identity_pipeline, static_cast<std::uint32_t>(entity), transform.compute());
	}
}

auto Scene::get_final_image() const -> const Disarray::Image& { return scene_renderer->get_composite_pass_image(); }

void Scene::draw_geometry()
{
	{
		auto point_light_view = registry.view<const Components::PointLight, const Components::Mesh, const Components::Pipeline>();
		static const Disarray::VertexBuffer* vertex_buffer = nullptr;
		static const Disarray::IndexBuffer* index_buffer = nullptr;
		static const Disarray::Pipeline* point_light_pipeline = nullptr;

		for (auto&& [entity, point_light, mesh, pipeline] : point_light_view.each()) {
			if (vertex_buffer == nullptr || index_buffer == nullptr || point_light_pipeline == nullptr) {
				vertex_buffer = &mesh.mesh->get_vertices();
				index_buffer = &mesh.mesh->get_indices();
				point_light_pipeline = pipeline.pipeline.get();
				break;
			}
		}

		scene_renderer->draw_mesh_instanced(*command_executor, count_point_lights, *vertex_buffer, *index_buffer, *point_light_pipeline);
	}

	for (auto line_view = registry.view<const Components::LineGeometry, const Components::Texture, const Components::Transform>();
		 auto&& [entity, geom, tex, transform] : line_view.each()) {
		scene_renderer->draw_planar_geometry(Geometry::Line,
			{
				.position = transform.position,
				.to_position = geom.to_position,
				.colour = tex.colour,
				.identifier = static_cast<std::uint32_t>(entity),
			});
	}

	for (auto rect_view = registry.view<const Components::Texture, const Components::QuadGeometry, const Components::Transform>();
		 auto&& [entity, tex, geom, transform] : rect_view.each()) {
		scene_renderer->draw_planar_geometry(Geometry::Rectangle,
			{
				.position = transform.position,
				.colour = tex.colour,
				.rotation = transform.rotation,
				.dimensions = transform.scale,
				.identifier = static_cast<std::uint32_t>(entity),
			});
	}

	for (auto&& [entity, transform, texture, light, mesh, pipeline] :
		registry
			.view<Components::Transform, const Components::Texture, const Components::DirectionalLight, const Components::Mesh,
				const Components::Pipeline>()
			.each()) {
		const auto identifier = static_cast<std::uint32_t>(entity);

		const auto look_at = glm::lookAt(transform.position, transform.position + glm::normalize(glm::vec3(light.direction)), { 0, 1, 0 });
		glm::quat rotation { look_at };
		transform.rotation = rotation;
		scene_renderer->draw_mesh(*command_executor, *mesh.mesh, *pipeline.pipeline, *texture.texture, transform.compute(), identifier);
		scene_renderer->draw_planar_geometry(Geometry::Line,
			{
				.position = transform.position,
				.to_position = { 0, 0, 0 },
				.colour = { 1, 0, 0, 1 },
				.identifier = identifier,
			});
	}

	for (auto mesh_view = registry.view<const Components::Mesh, const Components::Pipeline, const Components::Texture, const Components::Transform>(
			 entt::exclude<Components::PointLight>);
		 auto&& [entity, mesh, pipeline, texture, transform] : mesh_view.each()) {
		const auto identifier = static_cast<std::uint32_t>(entity);
		const auto& actual_pipeline = *pipeline.pipeline;
		if (mesh.draw_aabb) {
			scene_renderer->draw_aabb(*command_executor, mesh.mesh->get_aabb(), texture.colour, transform.compute());
		}
		if (mesh.mesh->has_children()) {
			scene_renderer->draw_submeshes(
				*command_executor, *mesh.mesh, actual_pipeline, *texture.texture, texture.colour, transform.compute(), identifier);
		} else {
			scene_renderer->draw_mesh(
				*command_executor, *mesh.mesh, actual_pipeline, *texture.texture, texture.colour, transform.compute(), identifier);
		}
	}

	for (auto&& [entity, mesh, pipeline, transform] :
		registry
			.view<const Components::Mesh, const Components::Pipeline, const Components::Transform>(
				entt::exclude<Components::Texture, Components::DirectionalLight, Components::PointLight>)
			.each()) {
		const auto identifier = static_cast<std::uint32_t>(entity);
		const auto& actual_pipeline = *pipeline.pipeline;
		const auto transform_computed = transform.compute();
		scene_renderer->draw_mesh(*command_executor, *mesh.mesh, actual_pipeline, transform_computed, identifier);
		if (mesh.draw_aabb) {
			scene_renderer->draw_aabb(*command_executor, mesh.mesh->get_aabb(), { 1, 1, 1, 1 }, transform_computed);
		}
	}
}

void Scene::draw_shadows()
{
	{
		auto point_light_view = registry.view<const Components::PointLight, const Components::Mesh, const Components::Pipeline>();
		static const Disarray::VertexBuffer* vertex_buffer = nullptr;
		static const Disarray::IndexBuffer* index_buffer = nullptr;

		for (auto&& [entity, point_light, mesh, pipeline] : point_light_view.each()) {
			if (vertex_buffer == nullptr || index_buffer == nullptr) {
				vertex_buffer = &mesh.mesh->get_vertices();
				index_buffer = &mesh.mesh->get_indices();
				break;
			}
		}

		scene_renderer->draw_mesh_instanced(*command_executor, count_point_lights, *vertex_buffer, *index_buffer, *shadow_instances_pipeline);
	}

	for (auto&& [entity, mesh, pipeline, texture, transform] :
		registry
			.view<const Components::Mesh, const Components::Pipeline, const Components::Texture, const Components::Transform>(
				entt::exclude<Components::PointLight>)
			.each()) {
		const auto identifier = static_cast<std::uint32_t>(entity);
		const auto& actual_pipeline = *shadow_pipeline;
		if (mesh.mesh->has_children()) {
			scene_renderer->draw_submeshes(
				*command_executor, *mesh.mesh, actual_pipeline, *texture.texture, texture.colour, transform.compute(), identifier);
		} else {
			scene_renderer->draw_mesh(
				*command_executor, *mesh.mesh, actual_pipeline, *texture.texture, texture.colour, transform.compute(), identifier);
		}
	}

	for (auto&& [entity, mesh, pipeline, transform] :
		registry
			.view<const Components::Mesh, const Components::Pipeline, const Components::Transform>(
				entt::exclude<Components::Texture, Components::DirectionalLight, Components::PointLight>)
			.each()) {
		const auto identifier = static_cast<std::uint32_t>(entity);
		const auto& actual_pipeline = *shadow_pipeline;
		const auto transform_computed = transform.compute();
		scene_renderer->draw_mesh(*command_executor, *mesh.mesh, actual_pipeline, transform_computed, identifier);
	}
}

void Scene::on_event(Event& event)
{
	EventDispatcher dispatcher { event };
	dispatcher.dispatch<KeyPressedEvent>([scene = this](KeyPressedEvent&) {
		if (Input::all<KeyCode::LeftControl, KeyCode::LeftShift, KeyCode::S>()) {
			SceneSerialiser scene_serialiser(scene);
		}
		return true;
	});
}

void Scene::recreate(const Extent& new_ex)
{
	extent = new_ex;

	for (auto&& [entity, pipeline] : registry.view<Components::Pipeline>().each()) {
		pipeline.pipeline->get_properties().extent = extent;
		pipeline.pipeline->get_properties().descriptor_set_layouts = scene_renderer->get_graphics_resource().get_descriptor_set_layouts();
	}

	framebuffers.at(SceneFramebuffer::Geometry)->recreate(true, new_ex);
	framebuffers.at(SceneFramebuffer::Identity)->recreate(true, new_ex);
	framebuffers.at(SceneFramebuffer::Shadow)->force_recreation();

	for (auto* pipe : { shadow_pipeline.get(), shadow_instances_pipeline.get(), identity_pipeline.get() }) {
		pipe->force_recreation();
	}
	command_executor->recreate(true, extent);
}

void Scene::destruct()
{
	SceneSerialiser scene_serialiser(this);
	for (auto&& [entity, script] : registry.view<Components::Script>().each()) {
		script.destroy();
	}

	file_watcher.reset();
	command_executor.reset();
}

auto Scene::create(std::string_view name) -> Entity
{
	auto entity = Entity(this, name);
	return entity;
}

void Scene::delete_entity(entt::entity entity) { registry.destroy(entity); }

void Scene::delete_entity(const Entity& entity) { delete_entity(entity.get_identifier()); }

auto Scene::deserialise(const Device& device, std::string_view name, const std::filesystem::path& filename) -> Scope<Scene>
{
	Scope<Scene> created = make_scope<Scene>(device, name);
	SceneDeserialiser deserialiser { *created, device, filename };
	return created;
}

auto Scene::deserialise_into(Scene& output_scene, const Device& device, const std::filesystem::path& filename) -> void
{
	SceneDeserialiser deserialiser { output_scene, device, filename };
}

void Scene::update_picked_entity(std::uint32_t handle) { picked_entity = make_scope<Entity>(this, handle == 0 ? entt::null : handle); }

void Scene::manipulate_entity_transform(Entity& entity, Camera& camera, GizmoType gizmo_type)
{
	ImGuizmo::SetDrawlist(nullptr);
	ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

	const auto& camera_view = camera.get_view_matrix();
	const auto& camera_projection = camera.get_projection_matrix();
	auto copy = camera_projection;
	copy[1][1] *= -1;

	auto& entity_transform = entity.get_components<Components::Transform>();
	auto transform = entity_transform.compute();

	bool snap = Input::key_pressed(KeyCode::LeftShift);
	float snap_value = 0.5F;
	if (gizmo_type == GizmoType::Rotate) {
		snap_value = 20.0F;
	}

	std::array<float, 3> snap_values = { snap_value, snap_value, snap_value };

	ImGuizmo::Manipulate(glm::value_ptr(camera_view), glm::value_ptr(copy), static_cast<ImGuizmo::OPERATION>(gizmo_type), ImGuizmo::LOCAL,
		glm::value_ptr(transform), nullptr, snap ? snap_values.data() : nullptr);

	if (ImGuizmo::IsUsing()) {
		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(transform, scale, rotation, translation, skew, perspective);

		auto delta_rotation = rotation - entity_transform.rotation;

		if (translation != glm::vec3 { 0, 0, 0 }) {
			entity_transform.position = translation;
		}

		entity_transform.rotation += delta_rotation;

		if (glm::l2Norm(scale) > 0) {
			entity_transform.scale = scale;
		}
	}
}

auto Scene::get_by_identifier(Identifier identifier) -> std::optional<Entity>
{
	for (const auto& [entity, id] : registry.view<Components::ID>().each()) {
		if (id.identifier == identifier) {
			return Entity { this, entity };
		}
	}

	return std::nullopt;
}

void Scene::clear()
{
	std::scoped_lock lock { registry_access };
	registry.clear();
}

} // namespace Disarray
