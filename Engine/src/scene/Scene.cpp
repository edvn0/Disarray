#include "DisarrayPCH.hpp"

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
	: device(dev)
	, scene_name(name)
{
	picked_entity = make_scope<Entity>(this);
	selected_entity = make_scope<Entity>(this);
}

void Scene::construct(Disarray::App& app) { extent = app.get_swapchain().get_extent(); }

Scene::~Scene() = default;

void Scene::begin_frame(const Camera& camera, SceneRenderer& scene_renderer)
{
	auto find_one = get_by_components<Components::Camera>();
	bool camera_component_usable = true;
	if (find_one) {
		auto&& [camera_component, transform] = find_one->get_components<Components::Camera, const Components::Transform>();
		// if we have a primary camera, then camera_component_usable should be false
		if (camera_component.is_primary) {
			auto&& [view, projection, pre_mul] = camera_component.compute(transform, extent);
			scene_renderer.begin_frame(view, projection, pre_mul);
		} else {
			camera_component_usable = false;
		}
	}

	if (!camera_component_usable || !find_one) {
		scene_renderer.begin_frame(camera.get_view_matrix(), camera.get_projection_matrix(), camera.get_view_projection());
	}

	auto [ubo, camera_ubo, light_ubos, shadow_pass, directional, glyph] = scene_renderer.get_graphics_resource().get_editable_ubos();
	auto& push_constant = scene_renderer.get_graphics_resource().get_editable_push_constant();

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
		const auto view = glm::lookAt(glm::vec3(transform.position), center, { 0.0F, 1.0F, 0.0F });

		const auto view_projection = projection * view;

		shadow_pass.view = view;
		shadow_pass.projection = projection;
		shadow_pass.view_projection = view_projection;
	}

	std::size_t light_index { 0 };
	auto& lights = light_ubos.lights;
	auto point_light_ssbo = scene_renderer.get_point_light_transforms().get_mutable<glm::mat4>();
	auto point_light_ssbo_colour = scene_renderer.get_point_light_colours().get_mutable<glm::vec4>();
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
	push_constant.max_point_lights = static_cast<std::uint32_t>(light_index);

	scene_renderer.get_graphics_resource().update_ubo();
}

void Scene::end_frame(SceneRenderer& renderer) { renderer.end_frame(); }

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
	auto [ubo, camera_ubo, light_ubos, shadow_pass, directional, glyph] = scene_renderer.get_graphics_resource().get_editable_ubos();
	camera_ubo.view = glm::mat3 { ubo.view };
	scene_renderer.get_graphics_resource().update_ubo(UBOIdentifier::Camera);
	auto skybox_view = registry.view<const Components::Skybox, const Components::Mesh>();
	Ref<Disarray::Mesh> skybox_ptr = nullptr;
	for (auto&& [entity, skybox, mesh] : skybox_view.each()) {
		if (mesh.mesh == nullptr) {
			continue;
		}
		skybox_ptr = mesh.mesh;
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
		const auto& pipeline = *scene_renderer.get_pipeline("PointLight");
		scene_renderer.draw_point_lights(*point_light_mesh, point_light_view_for_count, pipeline);
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

		const auto identifier = static_cast<std::uint32_t>(entity);
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

		const auto identifier = static_cast<std::uint32_t>(entity);
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
	{
		auto point_light_view = registry.view<const Components::PointLight, const Components::Mesh>();
		Ref<Disarray::Mesh> point_light_ptr = nullptr;
		for (auto&& [entity, point_light, mesh] : point_light_view.each()) {
			if (mesh.mesh->invalid())
				continue;
			point_light_ptr = mesh.mesh;
			break;
		}

		auto point_light_view_for_count = registry.view<const Components::PointLight>().size();
		const auto& shadow_pipeline = *scene_renderer.get_pipeline("ShadowInstances");
		scene_renderer.draw_point_lights(*point_light_ptr, point_light_view_for_count, shadow_pipeline);
	}

	for (auto&& [entity, mesh, texture, transform] :
		registry
			.view<const Components::Mesh, Components::Texture, const Components::Transform>(entt::exclude<Components::PointLight, Components::Skybox>)
			.each()) {
		if (mesh.mesh == nullptr) {
			continue;
		}

		const auto identifier = static_cast<std::uint32_t>(entity);
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
				entt::exclude<Components::Texture, Components::DirectionalLight, Components::PointLight, Components::Skybox>)
			.each()) {
		if (mesh.mesh == nullptr) {
			continue;
		}

		const auto identifier = static_cast<std::uint32_t>(entity);
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
		}
		return true;
	});
}

void Scene::recreate(const Extent& new_ex) { extent = new_ex; }

void Scene::destruct()
{
	SceneSerialiser scene_serialiser(this);
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
	template <ValidComponent Component> constexpr auto copy_component(auto& identifier_map, auto& old_reg)
	{
		for (auto&& [_, identifier, component] : old_reg.template view<Components::ID, Component>().each()) {
			Entity& destination_entity = identifier_map.at(identifier.get_id());
			destination_entity.put_component<Component>(component);
		}
	};
} // namespace

auto Scene::copy(Scene& scene) -> Scope<Scene>
{
	static constexpr auto copy_all
		= []<ValidComponent... C>(ComponentGroup<C...>, auto& identifier_map, auto& old_reg) { (copy_component<C>(identifier_map, old_reg), ...); };

	Scope<Scene> new_scene = make_scope<Scene>(scene.get_device(), scene.get_name());

	auto& old_registry = scene.get_registry();

	std::unordered_map<Identifier, Entity> identifiers {};
	for (auto&& [entity, identifier, tag] : old_registry.view<const Components::ID, const Components::Tag>().each()) {
		auto created = new_scene->create(tag.name);
		created.get_components<Components::ID>().identifier = identifier.identifier;
		identifiers.try_emplace(identifier.get_id(), std::move(created));
	}

	using CopyableComponents
		= ComponentGroup<Components::Transform, Components::Tag, Components::Inheritance, Components::LineGeometry, Components::QuadGeometry,
			Components::Mesh, Components::Material, Components::Texture, Components::DirectionalLight, Components::PointLight, Components::Controller,
			Components::Camera, Components::BoxCollider, Components::SphereCollider, Components::PillCollider, Components::Skybox, Components::Text>;
	copy_all(CopyableComponents {}, identifiers, old_registry);

	new_scene->sort<Components::ID>([](const Components::ID& left, const Components::ID& right) { return left.identifier < right.identifier; });

	new_scene->extent = scene.extent;
	new_scene->picked_entity.swap(scene.picked_entity);
	new_scene->selected_entity.swap(scene.selected_entity);

	return new_scene;
}

} // namespace Disarray
