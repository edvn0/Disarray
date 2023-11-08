#include "DisarrayPCH.hpp"

#include "Forward.hpp"

#include <glm/ext/matrix_clip_space.hpp>
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
#include "core/Instrumentation.hpp"
#include "core/Random.hpp"
#include "core/ThreadPool.hpp"
#include "core/events/Event.hpp"
#include "core/events/KeyEvent.hpp"
#include "core/events/MouseEvent.hpp"
#include "core/filesystem/AssetLocations.hpp"
#include "graphics/RendererProperties.hpp"
#include "physics/PhysicsEngine.hpp"
#include "scene/Camera.hpp"
#include "scene/Components.hpp"
#include "scene/CppScript.hpp"
#include "scene/Deserialiser.hpp"
#include "scene/Entity.hpp"
#include "scene/Scene.hpp"
#include "scene/Scripts.hpp"
#include "scene/Serialiser.hpp"
#include "ui/UI.hpp"

namespace Disarray {

Scene::Scene(const Device& dev, std::string_view name)
	: engine(6, 3)
	, device(dev)
	, scene_name(name)
{
	picked_entity = make_scope<Entity>(this);
	selected_entity = make_scope<Entity>(this);
}

void Scene::construct(Disarray::App& app) { extent = app.get_swapchain().get_extent(); }

Scene::~Scene() = default;

void Scene::execute_callbacks(SceneRenderer& renderer)
{
	while (!frame_start_callbacks.empty()) {
		auto&& front = frame_start_callbacks.front();
		frame_start_callbacks.pop();

		front(*this, renderer);
	}
}

void Scene::begin_frame(const Camera& camera, SceneRenderer& scene_renderer)
{
	if (const auto& view_projection_tuple = get_primary_camera(); view_projection_tuple.has_value()) {
		auto&& [view, proj, view_proj] = *view_projection_tuple;
		begin_frame(view, proj, view_proj, scene_renderer);
	} else {
		begin_frame(camera.get_view_matrix(), camera.get_projection_matrix(), camera.get_view_projection(), scene_renderer);
	}
}

void Scene::begin_frame(const glm::mat4& view, const glm::mat4& proj, const glm::mat4& view_proj, SceneRenderer& scene_renderer)
{
	execute_callbacks(scene_renderer);

	scene_renderer.begin_frame(view, proj, view_proj);

	auto directional_transaction = scene_renderer.begin_uniform_transaction<DirectionalLightUBO>();
	auto point_lights_transaction = scene_renderer.begin_uniform_transaction<PointLights>();
	auto spot_lights_transaction = scene_renderer.begin_uniform_transaction<SpotLights>();
	auto shadow_pass_transaction = scene_renderer.begin_uniform_transaction<ShadowPassUBO>();

	auto& shadow_pass = shadow_pass_transaction.get_buffer();
	auto& directional = directional_transaction.get_buffer();
	auto& point_lights = point_lights_transaction.get_buffer();
	auto& spot_lights = spot_lights_transaction.get_buffer();

	auto& push_constant = scene_renderer.get_graphics_resource().get_editable_push_constant();

	{
		for (auto sun_component_view = registry.view<const Components::Transform, Components::DirectionalLight>();
			 auto&& [entity, transform, sun] : sun_component_view.each()) {
			directional.position = { transform.position, 1.0F };
			sun.direction = glm::normalize(-directional.position); // Lookat {0,0,0};
			directional.direction = sun.direction;
			directional.ambient = sun.ambient;
			directional.diffuse = sun.diffuse;
			directional.specular = sun.specular;
			directional.near_far = glm::vec4 { 0 };
		}
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
		const auto shadow_pass_view = glm::lookAt(glm::vec3(transform.position), center, { 0.0F, 1.0F, 0.0F });

		const auto view_projection = projection * shadow_pass_view;

		shadow_pass.view = shadow_pass_view;
		shadow_pass.projection = projection;
		shadow_pass.view_projection = view_projection;
	}

	std::size_t point_light_index { 0 };
	auto& lights = point_lights.lights;
	auto point_light_ssbo = scene_renderer.get_point_light_transforms().get_mutable<glm::mat4>();
	auto point_light_ssbo_colour = scene_renderer.get_point_light_colours().get_mutable<glm::vec4>();
	for (auto&& [entity, point_light, pos, texture] :
		registry.view<const Components::PointLight, const Components::Transform, Components::Texture>().each()) {
		auto& light = lights.at(point_light_index);
		light.position = glm::vec4 { pos.position, 0.F };
		light.ambient = point_light.ambient;
		light.diffuse = point_light.diffuse;
		light.specular = point_light.specular;
		light.factors = point_light.factors;
		texture.colour = light.ambient;

		point_light_ssbo[point_light_index] = pos.compute();
		point_light_ssbo_colour[point_light_index] = texture.colour;
		point_light_index++;
	}
	push_constant.max_point_lights = static_cast<std::uint32_t>(point_light_index);

	std::size_t spot_light_index { 0 };
	auto& spot_light_array = spot_lights.lights;
	auto spot_light_ssbo = scene_renderer.get_spot_light_transforms().get_mutable<glm::mat4>();
	auto spot_light_ssbo_colour = scene_renderer.get_spot_light_colours().get_mutable<glm::vec4>();
	for (auto&& [entity, spot_light, pos, texture] :
		registry.view<const Components::SpotLight, Components::Transform, Components::Texture>().each()) {
		auto& light = spot_light_array.at(spot_light_index);
		pos.rotation = glm::quat { spot_light.direction };
		light.position = glm::vec4 { pos.position, 0.F };
		light.ambient = spot_light.ambient;
		light.diffuse = spot_light.diffuse;
		light.specular = spot_light.specular;
		light.direction_and_cutoff = {
			spot_light.direction,
			glm::cos(glm::radians(spot_light.cutoff_angle_degrees)),
		};
		light.factors = spot_light.factors;
		texture.colour = light.ambient;

		spot_light_ssbo[spot_light_index] = pos.compute();
		spot_light_ssbo_colour[spot_light_index] = texture.colour;
		spot_light_index++;
	}
	push_constant.max_spot_lights = static_cast<std::uint32_t>(spot_light_index);

	std::size_t identifier_index { 0 };
	auto ssbo_identifiers = scene_renderer.get_entity_identifiers().get_mutable<std::uint32_t>();
	auto identifiers_transforms = scene_renderer.get_entity_transforms().get_mutable<glm::mat4>();
	for (auto&& [entity, transform, id] : registry.view<const Components::Transform, const Components::ID>().each()) {
		if (!id.can_interact_with) {
			continue;
		}
		ssbo_identifiers[identifier_index] = static_cast<std::uint32_t>(entity);
		identifiers_transforms[identifier_index] = transform.compute();
		identifier_index++;
	}
}

void Scene::end_frame(SceneRenderer& renderer) { renderer.end_frame(); }

void Scene::interface()
{
	auto script_view = registry.view<Components::Script>();
	for (auto&& [entity, script] : script_view.each()) {
		script.get_script().on_interface();
	}
}

void Scene::update(float)
{
	if (picked_entity) {
		selected_entity.swap(picked_entity);
		picked_entity = nullptr;
	}
}

void Scene::render(SceneRenderer& renderer)
{
	auto render_planar_geometry = [](auto& ren) { ren.planar_geometry_pass(); };

	auto render_text = [](auto& ren, auto& reg) {
		for (auto&& [entity, text, transform] : reg.template view<const Components::Text, const Components::Transform>().each()) {
			if (text.projection == Components::TextProjection::ScreenSpace) {
				ren.draw_text(text.text_data, glm::uvec2(transform.position), text.size, text.colour);
			} else {
				ren.draw_text(text.text_data, transform.compute(), text.size, text.colour);
			}
		}

		ren.text_rendering_pass();
	};

	{
		// Skybox pass
		renderer.begin_pass<SceneFramebuffer::Geometry>(true);
		draw_skybox(renderer);
		renderer.end_pass();
	}
	{
		// Shadow pass
		renderer.begin_pass<SceneFramebuffer::Shadow>();
		draw_shadows(renderer);
		render_planar_geometry(renderer);
		renderer.end_pass();
	}
	{
		// Geometry pass
		renderer.begin_pass<SceneFramebuffer::Geometry>();
		draw_geometry(renderer);
		render_planar_geometry(renderer);
		renderer.end_pass();
	}
	{
		renderer.begin_pass<SceneFramebuffer::Identity>();
		draw_identifiers(renderer);
		renderer.end_pass();
	}
	{
		render_text(renderer, registry);
	}
	{
		// This is the composite pass!
		renderer.fullscreen_quad_pass();
	}
}

void Scene::draw_identifiers(SceneRenderer& scene_renderer)
{
	std::size_t count { 0 };
	for (auto&& [entity, id] : registry.view<Components::ID>().each()) {
		count += id.can_interact_with ? 1 : 0;
	}
	scene_renderer.draw_identifiers(count);
}

void Scene::draw_skybox(SceneRenderer& scene_renderer)
{
	auto skybox_view = registry.view<const Components::Skybox, const Components::Mesh>();
	Ref<Disarray::Mesh> skybox_ptr = nullptr;
	for (auto&& [entity, skybox, mesh] : skybox_view.each()) {
		if (mesh.mesh == nullptr) {
			continue;
		}
		skybox_ptr = mesh.mesh;
	}
	if (skybox_ptr == nullptr) {
		return;
	}

	scene_renderer.draw_skybox(*skybox_ptr);
}

void Scene::draw_geometry(SceneRenderer& scene_renderer)
{
	{
		auto point_light_view = registry.view<const Components::PointLight, const Components::Mesh>();
		Ref<Disarray::Mesh> point_light_mesh = nullptr;

		for (auto&& [entity, point_light, mesh] : point_light_view.each()) {
			point_light_mesh = mesh.mesh;
			break;
		}

		auto point_light_view_for_count = registry.view<const Components::PointLight>().size();
		if (point_light_view_for_count > 0 && point_light_mesh != nullptr) {
			const auto& pipeline = *scene_renderer.get_pipeline("PointLight");
			scene_renderer.draw_point_lights(*point_light_mesh, point_light_view_for_count, pipeline);
		}
	}

	{
		auto spot_light_view = registry.view<const Components::SpotLight, const Components::Mesh>();
		Ref<Disarray::Mesh> spot_light_mesh = nullptr;

		for (auto&& [entity, point_light, mesh] : spot_light_view.each()) {
			spot_light_mesh = mesh.mesh;
			break;
		}

		auto spot_light_view_for_count = registry.view<const Components::SpotLight>().size();
		if (spot_light_view_for_count > 0 && spot_light_mesh != nullptr) {
			const auto& pipeline = *scene_renderer.get_pipeline("SpotLight");
			scene_renderer.draw_point_lights(*spot_light_mesh, spot_light_view_for_count, pipeline);
		}
	}

	for (auto line_view = registry.view<const Components::LineGeometry, const Components::Texture, const Components::Transform>();
		 auto&& [entity, geom, tex, transform] : line_view.each()) {
		scene_renderer.draw_planar_geometry(Geometry::Line,
			{
				.position = transform.position,
				.to_position = geom.to_position,
				.colour = tex.colour,
				.identifier = static_cast<std::uint32_t>(entity),
			});
	}

	for (auto line_view = registry.view<const Components::LineGeometry, const Components::Transform>();
		 auto&& [entity, geom, transform] : line_view.each()) {
		scene_renderer.draw_planar_geometry(Geometry::Line,
			{
				.position = transform.position,
				.to_position = geom.to_position,
				.colour = { 0.9F, 0.2F, 0.6F, 1.0F },
				.identifier = static_cast<std::uint32_t>(entity),
			});
	}

	for (auto rect_view = registry.view<const Components::Texture, const Components::QuadGeometry, const Components::Transform>();
		 auto&& [entity, tex, geom, transform] : rect_view.each()) {
		scene_renderer.draw_planar_geometry(Geometry::Rectangle,
			{
				.position = transform.position,
				.colour = tex.colour,
				.rotation = transform.rotation,
				.dimensions = transform.scale,
				.identifier = static_cast<std::uint32_t>(entity),
			});
	}

	for (auto mesh_view = registry.view<const Components::Mesh, const Components::Texture, const Components::Transform>(
			 entt::exclude<Components::PointLight, Components::DirectionalLight, Components::Skybox>);
		 auto&& [entity, mesh, texture, transform] : mesh_view.each()) {

		if (mesh.mesh == nullptr) {
			continue;
		}

		const auto& actual_pipeline = *scene_renderer.get_pipeline("StaticMesh");
		const auto computed_transform = transform.compute();
		if (mesh.draw_aabb) {
			scene_renderer.draw_aabb(mesh.mesh->get_aabb(), texture.colour, computed_transform);
		}
		if (mesh.mesh->has_children()) {
			scene_renderer.draw_static_submeshes(mesh.mesh->get_submeshes(), actual_pipeline, computed_transform, texture.colour);
		} else {
			scene_renderer.draw_single_static_mesh(*mesh.mesh, actual_pipeline, computed_transform, texture.colour);
		}
	}

	for (auto&& [entity, mesh, transform] :
		registry
			.view<const Components::Mesh, const Components::Transform>(
				entt::exclude<Components::Texture, Components::DirectionalLight, Components::PointLight, Components::Skybox>)
			.each()) {
		if (mesh.mesh == nullptr) {
			continue;
		}

		const auto& actual_pipeline = *scene_renderer.get_pipeline("StaticMesh");
		const auto transform_computed = transform.compute();
		scene_renderer.draw_single_static_mesh(*mesh.mesh, actual_pipeline, transform_computed, { 1, 1, 1, 1 });
		if (mesh.draw_aabb) {
			scene_renderer.draw_aabb(mesh.mesh->get_aabb(), { 1, 1, 1, 1 }, transform_computed);
		}
	}
}

void Scene::draw_shadows(SceneRenderer& scene_renderer)
{
	for (auto&& [entity, mesh, texture, transform] : registry
														 .view<const Components::Mesh, Components::Texture, const Components::Transform>(
															 entt::exclude<Components::PointLight, Components::SpotLight, Components::Skybox>)
														 .each()) {
		if (mesh.mesh == nullptr) {
			continue;
		}

		const auto& actual_pipeline = *scene_renderer.get_pipeline("Shadow");
		if (mesh.mesh->has_children()) {
			scene_renderer.draw_static_submeshes(mesh.mesh->get_submeshes(), actual_pipeline, transform.compute(), texture.colour);
		} else {
			scene_renderer.draw_single_static_mesh(*mesh.mesh, actual_pipeline, transform.compute(), texture.colour);
		}
	}

	for (auto&& [entity, mesh, transform] :
		registry
			.view<const Components::Mesh, const Components::Transform>(
				entt::exclude<Components::Texture, Components::DirectionalLight, Components::SpotLight, Components::PointLight, Components::Skybox>)
			.each()) {
		if (mesh.mesh == nullptr) {
			continue;
		}

		const auto& actual_pipeline = *scene_renderer.get_pipeline("Shadow");
		const auto transform_computed = transform.compute();
		scene_renderer.draw_single_static_mesh(*mesh.mesh, actual_pipeline, transform_computed, { 1, 1, 1, 1 });
	}
}

void Scene::on_event(Event& event)
{
	EventDispatcher dispatcher { event };
	dispatcher.dispatch<KeyPressedEvent>([scene = this](KeyPressedEvent&) {
		if (Input::all<KeyCode::LeftControl, KeyCode::LeftShift, KeyCode::S>()) {
			SceneSerialiser scene_serialiser(scene);
			return true;
		}

		if (Input::all<KeyCode::LeftControl, KeyCode::D>()) {
			if (const auto& selected = scene->get_selected_entity(); selected != nullptr && !selected->is_valid()) {
				return false;
			}

			Scene::copy_entity(*scene, *scene->selected_entity);
			return true;
		}

		return false;
	});
}

void Scene::recreate(const Extent& new_ex) { extent = new_ex; }

void Scene::destruct()
{
	for (auto&& [entity, script] : registry.view<Components::Script>().each()) {
		script.destroy();
	}
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

	auto camera_view = camera.get_view_matrix();
	auto camera_projection = camera.get_projection_matrix();
	auto find_one = get_by_components<Components::Camera>();
	if (find_one) {
		auto&& [camera_component, transform] = find_one->get_components<Components::Camera, const Components::Transform>();
		// if we have a primary camera, then camera_component_usable should be false
		if (camera_component.is_primary) {
			auto&& [view, projection, pre_mul] = camera_component.compute(transform, extent);
			camera_view = view;
			camera_projection = projection;
		}
	}
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

void Scene::clear() { registry.clear(); }

namespace {
	template <ValidComponent C> constexpr auto copy_one(auto& copy_from, auto& copy_to)
	{
		if (copy_from.template has_component<C>()) {
			copy_to.template put_component<C>(copy_from.template get_components<C>());
		}
	}

	template <ValidComponent Component> constexpr auto copy_component(auto& identifier_map, auto& old_reg)
	{
		for (auto&& [_, identifier, component] : old_reg.template view<Components::ID, Component>().each()) {
			Entity& destination_entity = identifier_map.at(identifier.get_id());
			Component copy = component;
			destination_entity.put_component<Component>(copy);
		}
	}
} // namespace

auto Scene::copy(Scene& scene) -> Ref<Scene>
{
	static constexpr auto copy_all = []<ValidComponent... C>(Detail::ComponentGroup<C...>, auto& identifier_map, auto& old_reg) {
		(copy_component<C>(identifier_map, old_reg), ...);
	};

	auto new_scene = make_ref<Scene>(scene.get_device(), scene.get_name());

	auto& old_registry = scene.get_registry();

	std::unordered_map<Identifier, Entity> identifiers {};
	for (auto&& [entity, identifier, tag] : old_registry.view<const Components::ID, const Components::Tag>().each()) {
		auto created = new_scene->create(tag.name);
		created.get_components<Components::ID>().identifier = identifier.identifier;
		identifiers.try_emplace(identifier.get_id(), std::move(created));
	}

	using CopyableComponents = Detail::ComponentGroup<Components::Camera, Components::Transform, Components::Tag, Components::Inheritance,
		Components::LineGeometry, Components::QuadGeometry, Components::Mesh, Components::Material, Components::Texture, Components::DirectionalLight,
		Components::PointLight, Components::Controller, Components::BoxCollider, Components::SphereCollider, Components::CapsuleCollider,
		Components::ColliderMaterial, Components::Skybox, Components::Text, Components::RigidBody>;
	copy_all(CopyableComponents {}, identifiers, old_registry);

	new_scene->sort<Components::ID>([](const Components::ID& left, const Components::ID& right) { return left.identifier < right.identifier; });

	new_scene->extent = scene.extent;

	return new_scene;
}

auto Scene::copy_entity(Scene& scene, Entity& entity) -> void
{
	static std::unordered_map<Identifier, std::uint64_t> copies_of {};
	const auto& identifier = entity.get_components<Components::ID>().get_id();
	if (copies_of.contains(identifier)) {
		auto& count = copies_of.at(identifier);
		count++;
	} else {
		copies_of[identifier] = 1;
	}
	const auto& tag = entity.get_components<Components::Tag>().name;
	Scene::copy_entity(scene, entity, fmt::format("{}-Copy-{}", tag, copies_of.at(identifier)));
}

auto Scene::copy_entity(Scene& scene, Entity& to_copy_from_entity, std::string_view new_name) -> void
{
	static constexpr auto copy_all
		= []<ValidComponent... C>(Detail::ComponentGroup<C...>, auto& copy_from, auto& copy_to) { (copy_one<C>(copy_from, copy_to), ...); };

	auto new_entity = scene.create(new_name);
	using CopyableComponents = Detail::ComponentGroup<Components::Camera, Components::Transform, Components::Inheritance, Components::LineGeometry,
		Components::QuadGeometry, Components::Mesh, Components::Material, Components::Texture, Components::DirectionalLight, Components::PointLight,
		Components::Controller, Components::BoxCollider, Components::SphereCollider, Components::CapsuleCollider, Components::ColliderMaterial,
		Components::Skybox, Components::Text, Components::RigidBody>;

	copy_all(CopyableComponents {}, to_copy_from_entity, new_entity);
}

auto Scene::step(std::int32_t steps) -> void { step_frames = steps; }

auto Scene::on_runtime_start() -> void
{
	running = true;

	on_physics_start();

	// TODO(edvin): Scripting start
}

auto Scene::on_simulation_start() -> void { on_physics_start(); }

auto Scene::on_runtime_stop() -> void
{
	running = false;

	on_physics_stop();

	// TODO(edvin): Scripting end
}

auto Scene::on_simulation_stop() -> void { on_physics_stop(); }

auto Scene::get_primary_camera() -> std::optional<ViewProjectionTuple>
{
	auto find_one = get_by_components<Components::Camera>();
	if (find_one) {
		auto&& [camera_component, transform] = find_one->get_components<Components::Camera, const Components::Transform>();
		if (camera_component.is_primary) {
			return camera_component.compute(transform, extent);
		}
		return {};
	}

	return {};
}

auto Scene::on_update_editor(float time_step) -> void { update(time_step); }

static constexpr auto fixed_time_step = 1.0F / 60.F;

auto Scene::on_update_simulation(float time_step) -> void
{
	physics_update(fixed_time_step);
	update(time_step);
}

auto Scene::on_update_runtime(float time_step) -> void
{
	auto script_view = registry.view<Components::Script>();
	std::vector<CppScript*> deferred_scripts {};
	deferred_scripts.reserve(script_view.size());
	script_view.each([&](const auto entity, Components::Script& script) {
		if (script.has_been_bound()) {
			script.instantiate();
		}

		auto& instantiated = script.get_script();
		instantiated.update_entity(this, entity);
		deferred_scripts.push_back(&instantiated);
	});

	Collections::parallel_for_each(deferred_scripts, [step = time_step](auto* script) { script->on_update(step); });

	auto controller_view = registry.view<Components::Controller, Components::Transform>();
	controller_view.each([step = time_step](const auto, Components::Controller& controller, Components::Transform& transform) {
		controller.on_update(step, transform);
	});

	physics_update(fixed_time_step);
}

auto Scene::on_physics_start() -> void
{
	auto view = registry.view<Components::RigidBody>();
	for (auto e : view) {
		Entity entity { this, e };
		auto& transform = entity.get_components<Components::Transform>();
		auto& rigid_body = entity.get_components<Components::RigidBody>();
		auto* body = engine.create_rigid_body({
			.position = transform.position,
			.rotation = transform.rotation,
			.linear_drag = rigid_body.linear_drag,
			.angular_drag = rigid_body.angular_drag,
			.type = rigid_body.body_type,
			.mass = rigid_body.mass,
			.disable_gravity = rigid_body.disable_gravity,
			.is_kinematic = rigid_body.is_kinematic,
		});

		CollisionMaterial collision_material {};

		if (entity.has_component<Components::ColliderMaterial>()) {
			auto& material = entity.get_components<Components::ColliderMaterial>();
			collision_material.mass_density = material.mass_density;
			collision_material.friction_coefficient = material.friction_coefficient;
			collision_material.bounciness = material.bounciness;
		}

		if (entity.has_component<Components::CapsuleCollider>()) {
			auto& collider = entity.get_components<Components::CapsuleCollider>();
			engine.add_capsule_collider(body,
				{
					.radius = collider.radius,
					.height = collider.height,
					.offset = collider.offset,
					.collision_material = collision_material,
				});
		}
		if (entity.has_component<Components::BoxCollider>()) {
			auto& collider = entity.get_components<Components::BoxCollider>();
			engine.add_box_collider(body,
				{
					.half_size = transform.scale * 2.0F * collider.half_size,
					.offset = collider.offset,
					.collision_material = collision_material,
				});
		}
		if (entity.has_component<Components::SphereCollider>()) {
			auto& collider = entity.get_components<Components::SphereCollider>();
			engine.add_sphere_collider(body,
				{
					.radius = collider.radius,
					.offset = collider.offset,
					.collision_material = collision_material,
				});
		}
		rigid_body.engine_body_storage = body;
	}
}

auto Scene::on_physics_stop() -> void
{
	for (auto&& [entity, rigid] : registry.view<Components::RigidBody>().each()) {
		rigid.engine_body_storage = nullptr;
	}
	engine.restart();
}

auto Scene::sort() -> void
{
	sort<Components::ID>([](const auto& left, const auto& right) { return left.identifier < right.identifier; });
}

void Scene::physics_update(float time_step)
{
	if (!is_paused() || step_frames <= 0) {
		engine.step(time_step);
		auto view = registry.view<Components::RigidBody, Components::Transform>();
		for (auto&& [handle, rigid_body, transform] : view.each()) {

			auto&& [new_position, new_rotation] = engine.get_transform(rigid_body.engine_body_storage);
			transform.position.x = new_position.x;
			transform.position.y = new_position.y;
			transform.position.z = new_position.z;

			transform.rotation = new_rotation;
		}

		step_frames--;
		if (step_frames < 0) {
			step_frames = 0;
		}
	}
}

} // namespace Disarray
