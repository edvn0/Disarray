#include "DisarrayPCH.hpp"

#include "scene/Scene.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

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
#include "core/ThreadPool.hpp"
#include "core/events/Event.hpp"
#include "core/events/KeyEvent.hpp"
#include "core/events/MouseEvent.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Maths.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/PushConstantLayout.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/RendererProperties.hpp"
#include "graphics/ShaderCompiler.hpp"
#include "graphics/Texture.hpp"
#include "graphics/TextureCache.hpp"
#include "scene/Camera.hpp"
#include "scene/Components.hpp"
#include "scene/CppScript.hpp"
#include "scene/Deserialiser.hpp"
#include "scene/Entity.hpp"
#include "scene/Scripts.hpp"
#include "scene/Serialiser.hpp"

template <std::size_t Count> static consteval auto generate_colours() -> std::array<glm::vec4, Count>
{
	std::array<glm::vec4, Count> colours {};
	constexpr auto division = 1.F / static_cast<float>(Count);
	for (std::size_t i = 0; i < Count; i++) {
		colours.at(i) = glm::vec4 { division * i, division * i, 0.3, 1 };
	}
	return colours;
}

template <std::size_t Count> static consteval auto generate_angles() -> std::array<float, Count>
{
	std::array<float, Count> angles {};
	constexpr auto division = 1.F / static_cast<float>(Count);
	for (std::size_t i = 0; i < Count; i++) {
		angles.at(i) = glm::two_pi<float>() * division * i;
	}
	return angles;
}

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
	file_watcher = make_scope<FileWatcher>(pool, "Assets/Shaders", Collections::StringSet { ".vert", ".frag" }, std::chrono::milliseconds(200));
	file_watcher->on_modified([&dev = device, &reg = registry, &mutex = registry_access](const FileInformation& entry) {
		std::scoped_lock lock { mutex };
		const auto view = reg.view<Components::Pipeline>();
		std::unordered_set<Components::Pipeline*> unique_pipelines_sharing_this_files {};
		for (auto&& [ent, pipeline] : view.each()) {
			if (!pipeline.pipeline->has_shader_with_name(entry.to_path())) {
				continue;
			}
			unique_pipelines_sharing_this_files.insert(&pipeline);
		}

		if (unique_pipelines_sharing_this_files.empty()) {
			return;
		}

		Runtime::ShaderCompiler compiler;
		ShaderType type = to_shader_type(entry.path);
		auto&& [could, code] = compiler.try_compile(entry.path, type);
		if (!could) {
			return;
		}

		auto shader = Shader::construct(dev,
			{
				.code = std::move(code),
				.identifier = entry.path,
				.type = type,
			});

		for (auto* pipe : unique_pipelines_sharing_this_files) {
			auto& [pipeline] = *pipe;
			auto& pipe_props = pipeline->get_properties();

			switch (type) {
			case ShaderType::Vertex: {
				pipe_props.vertex_shader = shader;
			}
			case ShaderType::Fragment: {
				pipe_props.fragment_shader = shader;
			}
			case ShaderType::Compute: {
				pipe_props.compute_shader = shader;
			}
			}
		}

		wait_for_idle(dev);
		for (auto* comp : unique_pipelines_sharing_this_files) {
			comp->pipeline->force_recreation();
		}
		wait_for_idle(dev);
	});
}

void Scene::construct(Disarray::App& app, Disarray::Threading::ThreadPool& pool)
{
	scene_renderer = Renderer::construct_unique(device, app.get_swapchain(), {});

	setup_filewatcher_and_threadpool(pool);

	extent = app.get_swapchain().get_extent();
	command_executor = CommandExecutor::construct(device, &app.get_swapchain(), { .count = 3, .is_primary = true, .record_stats = true });
	scene_renderer->on_batch_full([&exec = *command_executor](Renderer& renderer) { renderer.flush_batch(exec); });

	framebuffer = Framebuffer::construct(device,
		{
			.extent = extent,
			.attachments = { { Disarray::ImageFormat::SBGR }, { ImageFormat::Depth } },
			.clear_colour_on_load = false,
			.clear_depth_on_load = false,
			.debug_name = "FirstFramebuffer",
		});
	shadow_framebuffer = Framebuffer::construct(device,
		{
			.extent = extent,
			.attachments = { { ImageFormat::Depth } },
			.clear_colour_on_load = true,
			.clear_depth_on_load = true,
			.debug_name = "ShadowFramebuffer",
		});

	const auto& resources = scene_renderer->get_graphics_resource();
	const auto& desc_layout = resources.get_descriptor_set_layouts();

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
			},
			.push_constant_layout = { { PushConstantKind::Both, sizeof(PushConstant) } },
			.write_depth = true,
			.test_depth = true,
			.descriptor_set_layouts = desc_layout,
		});
	identity_framebuffer = Framebuffer::construct(device,
		{
			.extent = extent,
			.attachments = { { ImageFormat::SBGR }, { ImageFormat::Uint }, { ImageFormat::Depth } },
			.clear_colour_on_load = true,
			.clear_depth_on_load = true,
			.debug_name = "IdentityFramebuffer",
		});

	create_entities();
}

Scene::~Scene() = default;

void Scene::begin_frame(const Camera& camera)
{
	scene_renderer->begin_frame(camera);
	auto [ubo, camera_ubo, light_ubos] = scene_renderer->get_graphics_resource().get_editable_ubos();
	auto& push_constant = scene_renderer->get_graphics_resource().get_editable_push_constant();

	for (auto sun_component_view = registry.view<const Components::DirectionalLight, const Components::Texture>();
		 auto&& [entity, sun, texture] : sun_component_view.each()) {
		ubo.sun_direction_and_intensity = sun.compute();
		ubo.sun_colour = texture.colour;
	}

	std::size_t light_index { 0 };
	auto& lights = light_ubos.lights;
	for (auto point_light_view = registry.view<const Components::PointLight, const Components::Transform, const Components::Texture>();
		 auto&& [entity, point_light, pos, texture] : point_light_view.each()) {
		auto& light = lights.at(light_index++);
		light.position = glm::vec4 { pos.position, 0.F };
		light.ambient = texture.colour;
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

void Scene::render()
{
	command_executor->begin();
#ifdef SHADOWPASS_SUPPORTED
	{
		scene_renderer->begin_pass(*command_executor, *shadow_framebuffer);
		draw_geometry(true);
		scene_renderer->end_pass(*command_executor);
	}
#endif
	{
		scene_renderer->begin_pass(*command_executor, *framebuffer, true);

		auto line_view = registry.view<const Components::LineGeometry, const Components::Texture, const Components::Transform>();
		for (auto&& [entity, geom, tex, transform] : line_view.each()) {
			scene_renderer->draw_planar_geometry(Geometry::Line,
				{
					.position = transform.position,
					.to_position = geom.to_position,
					.colour = tex.colour,
				});
		}

		scene_renderer->end_pass(*command_executor);
	}
	{
		scene_renderer->begin_pass(*command_executor, *identity_framebuffer, true);
		draw_geometry();
		scene_renderer->end_pass(*command_executor);
	}
	command_executor->submit_and_end();
}

void Scene::draw_geometry(bool is_shadow)
{
	for (auto script_view = registry.view<Components::Script>(); auto&& [entity, script] : script_view.each()) {
		script.get_script().on_render(*scene_renderer);
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

	for (auto rect_view
		 = registry.view<const Components::Texture, const Components::QuadGeometry, const Components::Transform, const Components::ID>();
		 auto&& [entity, tex, geom, transform, id] : rect_view.each()) {
		scene_renderer->draw_planar_geometry(Geometry::Rectangle,
			{
				.position = transform.position,
				.colour = tex.colour,
				.rotation = transform.rotation,
				.dimensions = transform.scale,
				.identifier = static_cast<std::uint32_t>(entity),
			});
	}

	for (auto mesh_view = registry.view<const Components::Mesh, const Components::Pipeline, const Components::Texture, const Components::Transform>();
		 auto&& [entity, mesh, pipeline, texture, transform] : mesh_view.each()) {
		const auto identifier = static_cast<std::uint32_t>(entity);
		if (mesh.mesh->has_children()) {
			scene_renderer->draw_submeshes(*command_executor, *mesh.mesh, is_shadow ? *shadow_pipeline : *pipeline.pipeline, *texture.texture,
				texture.colour, transform.compute(), identifier);
		} else {
			scene_renderer->draw_mesh(*command_executor, *mesh.mesh, is_shadow ? *shadow_pipeline : *pipeline.pipeline, *texture.texture,
				texture.colour, transform.compute(), identifier);
		}
	}

	for (auto mesh_no_texture_view = registry.view<const Components::Mesh, const Components::Pipeline, const Components::Transform>(
			 entt::exclude<Components::Texture, Components::DirectionalLight>);
		 auto&& [entity, mesh, pipeline, transform] : mesh_no_texture_view.each()) {
		const auto identifier = static_cast<std::uint32_t>(entity);
		scene_renderer->draw_mesh(*command_executor, *mesh.mesh, is_shadow ? *shadow_pipeline : *pipeline.pipeline, transform.compute(), identifier);
	}

	auto directional_lights
		= registry.view<Components::Transform, const Components::DirectionalLight, const Components::Mesh, const Components::Pipeline>();
	for (auto&& [entity, transform, light, mesh, pipeline] : directional_lights.each()) {
		const auto identifier = static_cast<std::uint32_t>(entity);

		const auto look_at = glm::lookAt(transform.position, transform.position + glm::normalize(light.direction), { 0, 1, 0 });
		glm::quat rotation { look_at };
		transform.rotation = rotation;
		scene_renderer->draw_mesh(*command_executor, *mesh.mesh, is_shadow ? *shadow_pipeline : *pipeline.pipeline, transform.compute(), identifier);
	}

	// TODO: Implement
	// scene_renderer->draw_text("Hello world!", 0, 0, 12.f);
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

	identity_framebuffer->recreate(true, extent);
	framebuffer->recreate(true, extent);
	command_executor->recreate(true, extent);
}

void Scene::destruct()
{
	SceneSerialiser scene_serialiser(this);
	for (auto scripts = registry.view<Components::Script>(); auto&& [entity, script] : scripts.each()) {
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
	float snap_value = 0.5f;
	if (static_cast<ImGuizmo::OPERATION>(gizmo_type) == ImGuizmo::OPERATION::ROTATE) {
		snap_value = 45.0f;
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

		entity_transform.position = translation;
		entity_transform.rotation += delta_rotation;
		entity_transform.scale = scale;
	}
}

auto Scene::get_by_identifier(Identifier identifier) -> std::optional<Entity>
{
	for (const auto [entity, id] : registry.view<Components::ID>().each()) {
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

void Scene::create_entities()
{
	VertexLayout layout {
		{ ElementType::Float3, "position" },
		{ ElementType::Float2, "uv" },
		{ ElementType::Float4, "colour" },
		{ ElementType::Float3, "normals" },
	};
	const auto& resources = scene_renderer->get_graphics_resource();
	const auto& desc_layout = resources.get_descriptor_set_layouts();

	{
		int rects { 2 };

		const auto& vert = scene_renderer->get_pipeline_cache().get_shader("cube.vert");
		const auto& frag = scene_renderer->get_pipeline_cache().get_shader("cube.frag");

		auto pipe = Pipeline::construct(device,
			{
				.vertex_shader = vert,
				.fragment_shader = frag,
				.framebuffer = identity_framebuffer,
				.layout = layout,
				.push_constant_layout = { { PushConstantKind::Both, sizeof(PushConstant) } },
				.extent = extent,
				.cull_mode = CullMode::Back,
				.descriptor_set_layouts = desc_layout,
			});

		const auto cube_mesh = Mesh::construct(device,
			MeshProperties {
				.path = "Assets/Models/cube.obj",
			});
		auto parent = create("Grid");
		for (auto j = -rects / 2; j < rects / 2; j++) {
			for (auto i = -rects / 2; i < rects / 2; i++) {
				auto rect = create(fmt::format("Rect{}-{}", i, j));
				parent.add_child(rect);
				auto& transform = rect.template get_components<Components::Transform>();
				transform.position = { 5 * static_cast<float>(i) + 2.5f, -1, 5 * static_cast<float>(j) + 2.5f };
				transform.rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3 { 1, 0, 0 });
				float col_x = (i + (static_cast<float>(rects) / 2)) / static_cast<float>(rects);
				float col_y = (j + (static_cast<float>(rects) / 2)) / static_cast<float>(rects);
				if (col_x == 0) {
					col_x += 0.2f;
				}

				if (col_y == 0) {
					col_y += 0.2f;
				}
				glm::vec4 colour { col_x, 0, col_y, 1 };
				rect.template add_component<Components::Mesh>(cube_mesh);
				rect.template add_component<Components::Texture>(colour);
				rect.template add_component<Components::Pipeline>(pipe);
			}
		}
	}

	{
		auto unit_vectors = create("UnitVectors");
		const glm::vec3 base_pos { 0, 0, 0 };
		{
			auto axis = create("XAxis");
			auto& transform = axis.get_components<Components::Transform>();
			transform.position = base_pos;
			axis.add_component<Components::LineGeometry>(base_pos + glm::vec3 { 10.0, 0, 0 });
			axis.add_component<Components::Texture>(glm::vec4 { 1, 0, 0, 1 });
			unit_vectors.add_child(axis);
		}
		{
			auto axis = create("YAxis");
			auto& transform = axis.get_components<Components::Transform>();
			transform.position = base_pos;
			axis.add_component<Components::LineGeometry>(base_pos + glm::vec3 { 0, -10.0, 0 });
			axis.add_component<Components::Texture>(glm::vec4 { 0, 1, 0, 1 });
			unit_vectors.add_child(axis);
		}
		{
			auto axis = create("ZAxis");
			auto& transform = axis.get_components<Components::Transform>();
			transform.position = base_pos;
			axis.add_component<Components::LineGeometry>(base_pos + glm::vec3 { 0, 0, -10.0 });
			axis.add_component<Components::Texture>(glm::vec4 { 0, 0, 1, 1 });
			unit_vectors.add_child(axis);
		}
	}

	{
		const auto& vert = scene_renderer->get_pipeline_cache().get_shader("sponza.vert");
		const auto& frag = scene_renderer->get_pipeline_cache().get_shader("sponza.frag");

		const auto rotation = Maths::rotate_by(glm::radians(glm::vec3 { 180, 0, 0 }));

		auto v_mesh = create("Sponza");
		auto viking = Mesh::construct(device,
			{
				.path = "Assets/Models/sponza/sponza.obj",
				.initial_rotation = rotation,
			});
		auto& mesh = v_mesh.add_component<Components::Mesh>(viking);
		const auto& textures = mesh.mesh->get_textures();
		(void)textures;

		Log::info("Scene", "Textures: {}", textures.size());

		v_mesh.get_components<Components::Transform>().scale = glm::vec3 { 0.1F };
		auto sponza_pipeline = Pipeline::construct(device,
			{
				.vertex_shader = vert,
				.fragment_shader = frag,
				.framebuffer = identity_framebuffer,
				.layout = layout,
				.push_constant_layout = { { PushConstantKind::Both, sizeof(PushConstant) } },
				.extent = extent,
				.polygon_mode = PolygonMode::Fill,
				.cull_mode = CullMode::Back,
				.descriptor_set_layouts = desc_layout,
			});
		v_mesh.add_component<Components::Pipeline>(sponza_pipeline);
		v_mesh.add_component<Components::Texture>();
		v_mesh.add_component<Components::Material>(Material::construct(device,
			{
				.vertex_shader = vert,
				.fragment_shader = frag,
			}));
	}

	{
		const auto& vert = scene_renderer->get_pipeline_cache().get_shader("main.vert");
		const auto& frag = scene_renderer->get_pipeline_cache().get_shader("main.frag");

		auto viking_rotation = Maths::rotate_by(glm::radians(glm::vec3 { 90, 0, 0 }));
		auto v_mesh = create("Viking");
		const auto viking = Mesh::construct(device,
			{
				.path = "Assets/Models/viking.obj",
				.initial_rotation = viking_rotation,
			});
		v_mesh.add_component<Components::Mesh>(viking);
		v_mesh.add_component<Components::Pipeline>(Pipeline::construct(device,
			{
				.vertex_shader = vert,
				.fragment_shader = frag,
				.framebuffer = identity_framebuffer,
				.layout = layout,
				.push_constant_layout = { { PushConstantKind::Both, sizeof(PushConstant) } },
				.extent = extent,
				.cull_mode = CullMode::Back,
				.descriptor_set_layouts = desc_layout,
			}));
		v_mesh.add_component<Components::Texture>(scene_renderer->get_texture_cache().get("viking_room"));
		v_mesh.add_component<Components::Material>(Material::construct(device,
			{
				.vertex_shader = vert,
				.fragment_shader = frag,
			}));

		TextureProperties texture_properties {
			.extent = extent,
			.format = ImageFormat::SBGR,
			.debug_name = "viking",
		};
		texture_properties.path = "Assets/Textures/viking_room.png";
		v_mesh.add_component<Components::Texture>(Texture::construct(device, texture_properties));
		static constexpr auto val = 10.0F;
		v_mesh.add_script<Scripts::LinearMovementScript>(-val, val);
	}

	{
		auto sun = create("Sun");

		const auto& vert = scene_renderer->get_pipeline_cache().get_shader("main.vert");
		const auto& frag = scene_renderer->get_pipeline_cache().get_shader("main.frag");

		sun.add_component<Components::Pipeline>(Pipeline::construct(device,
			{
				.vertex_shader = vert,
				.fragment_shader = frag,
				.framebuffer = identity_framebuffer,
				.layout = layout,
				.push_constant_layout = { { PushConstantKind::Both, sizeof(PushConstant) } },
				.extent = extent,
				.cull_mode = CullMode::Back,
				.descriptor_set_layouts = desc_layout,
			}));
		sun.add_component<Components::Texture>(nullptr, glm::vec4 { 0.6, 0.8, 0.1, 1.0 });
		sun.add_component<Components::DirectionalLight>();
		const auto mesh = Mesh::construct(device, MeshProperties { .path = "Assets/Models/arrow.obj" });
		sun.add_component<Components::Mesh>(mesh);
		sun.get_components<Components::Transform>().position = { 5, -5, 5 };
	}

	{
		static constexpr auto max_radius = 8U;
		static constexpr auto point_lights = 30U;

		constexpr auto colours = generate_colours<point_lights>();
		constexpr auto angles = generate_angles<point_lights>();

		const auto sphere = Mesh::construct(device, { .path = "Assets/Models/sphere.obj" });
		const auto& vert = scene_renderer->get_pipeline_cache().get_shader("point_light.vert");
		const auto& frag = scene_renderer->get_pipeline_cache().get_shader("point_light.frag");
		auto pipe = Pipeline::construct(device,
			{
				.vertex_shader = vert,
				.fragment_shader = frag,
				.framebuffer = identity_framebuffer,
				.layout = layout,
				.push_constant_layout = { { PushConstantKind::Both, sizeof(PushConstant) } },
				.extent = extent,
				.cull_mode = CullMode::Back,
				.descriptor_set_layouts = desc_layout,
			});
		auto pl_system = create("PointLightSystem");
		for (std::uint32_t i = 0; i < colours.size(); i++) {
			auto point_light = create("PointLight-{}", i);
			point_light.add_component<Components::PointLight>();
			auto& transform = point_light.get_components<Components::Transform>();
			const auto divided = angles.at(i);
			transform.position = { glm::sin(divided), 0, glm::cos(divided) };
			transform.position *= max_radius;
			transform.position.y = -4;
			transform.scale *= 0.2;

			point_light.add_component<Components::Mesh>(sphere);
			point_light.add_component<Components::Texture>(colours.at(i));
			point_light.add_component<Components::Pipeline>(pipe);
			pl_system.add_child(point_light);
		}
	}

	{
#define TEST_DESCRIPTOR_SETS
#ifdef TEST_DESCRIPTOR_SETS
		std::array<std::uint32_t, 1> white_tex_data = { 1 };
		DataBuffer pixels { white_tex_data.data(), sizeof(std::uint32_t) };
		TextureProperties white_tex_props {
			.extent = extent,
			.format = ImageFormat::SBGR,
			.debug_name = "white_tex",
		};
		Ref<Texture> white_tex = Texture::construct(device, white_tex_props);
		scene_renderer->get_graphics_resource().expose_to_shaders(*white_tex);
#endif
	}
}

} // namespace Disarray
