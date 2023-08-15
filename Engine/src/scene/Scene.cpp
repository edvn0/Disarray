#include "DisarrayPCH.hpp"

#include "core/App.hpp"
#include "core/Formatters.hpp"
#include "core/Input.hpp"
#include "core/ThreadPool.hpp"
#include "core/events/Event.hpp"
#include "core/events/KeyEvent.hpp"
#include "core/events/MouseEvent.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Texture.hpp"
#include "scene/Components.hpp"
#include "scene/Deserialiser.hpp"
#include "scene/Entity.hpp"
#include "scene/Scene.hpp"
#include "scene/Serialiser.hpp"

#include <ImGuizmo.h>
#include <entt/entt.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

static auto rotate_by(const glm::vec3& axis_radians)
{
	glm::mat4 identity = glm::identity<glm::mat4>();
	glm::rotate(identity, axis_radians.x, glm::vec3 { 1, 0, 0 });
	glm::rotate(identity, axis_radians.y, glm::vec3 { 0, 1, 0 });
	glm::rotate(identity, axis_radians.z, glm::vec3 { 0, 0, 1 });
	return identity;
}

namespace Disarray {

Scene::Scene(const Device& dev, std::string_view name)
	: device(dev)
	, scene_name(name)
	, registry(entt::basic_registry())
{
	picked_entity = make_scope<Entity>(*this);
	selected_entity = make_scope<Entity>(*this);
}

void Scene::construct(Disarray::App& app, Disarray::Renderer& renderer, Disarray::ThreadPool&)
{
	extent = app.get_swapchain().get_extent();
	command_executor = CommandExecutor::construct(device, app.get_swapchain(), { .count = 3, .is_primary = true, .record_stats = true });

	int rects_x { 2 };
	int rects_y { 2 };
	auto parent = create("Grid");
	for (auto j = -rects_y / 2; j < rects_y / 2; j++) {
		for (auto i = -rects_x / 2; i < rects_x / 2; i++) {
			auto rect = create(fmt::format("Rect{}-{}", i, j));
			parent.add_child(rect);
			auto& transform = rect.get_components<Components::Transform>();
			transform.position = { 2 * static_cast<float>(i) + 0.5f, -1, 2 * static_cast<float>(j) + 0.5f };
			transform.rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3 { 1, 0, 0 });
			glm::vec4 colour { i, 0, j, 1 };
			rect.add_component<Components::QuadGeometry>();
			rect.add_component<Components::Texture>(colour);
		}
	}

#ifdef FLOOR
	auto floor = create("Floor");
	floor.add_component<Components::Geometry>();
	floor.add_component<Components::Texture>(glm::vec4 { 0.2, 0.2, 0.8, 1.0f });
	floor.add_component<QuadGeometry>();
	auto& floor_transform = floor.get_components<Transform>();
	floor_transform.scale = { 100, 100, 1 };
	floor_transform.rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3 { 1, 0, 0 });
#endif

	auto unit_vectors = create("UnitVectors");
	{
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

	renderer.on_batch_full([&exec = *command_executor](Renderer& r) { r.flush_batch(exec); });

	VertexLayout layout {
		{ ElementType::Float3, "position" },
		{ ElementType::Float2, "uv" },
		{ ElementType::Float4, "colour" },
		{ ElementType::Float3, "normals" },
	};

	framebuffer = Framebuffer::construct(device,
		{
			.extent = extent,
			.attachments = { { Disarray::ImageFormat::SBGR }, { ImageFormat::Depth } },
			.clear_colour_on_load = false,
			.clear_depth_on_load = false,
			.debug_name = "FirstFramebuffer",
		});

	identity_framebuffer = Framebuffer::construct(device,
		{
			.extent = extent,
			.attachments = { { ImageFormat::SBGR }, { ImageFormat::Uint, false }, { ImageFormat::Depth } },
			.clear_colour_on_load = true,
			.clear_depth_on_load = true,
			.debug_name = "IdentityFramebuffer",
		});

	const auto& vert = renderer.get_pipeline_cache().get_shader("main.vert");
	const auto& frag = renderer.get_pipeline_cache().get_shader("main.frag");
	PipelineProperties props = {
			.vertex_shader = vert,
			.fragment_shader = frag,
			.framebuffer = framebuffer,
			.layout = layout,
			.push_constant_layout = PushConstantLayout { PushConstantRange { PushConstantKind::Both, std::size_t { 84 }, }, },
			.extent = extent,
			.depth_comparison_operator = DepthCompareOperator::GreaterOrEqual,
			.cull_mode = CullMode::Back,
			.descriptor_set_layouts = renderer.get_descriptor_set_layouts(),
		};
	auto viking_rotation = rotate_by(glm::radians(glm::vec3 { 0, 0, 90 }));

	auto v_mesh = create("Viking");
	v_mesh.add_component<Components::Mesh>(Mesh::construct(device,
		{
			.path = "Assets/Models/viking.mesh",
			.initial_rotation = viking_rotation,
		}));
	v_mesh.add_component<Components::Pipeline>(Pipeline::construct(device, props));
	v_mesh.add_component<Components::Texture>(renderer.get_texture_cache().get("viking_room"));
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

#define TEST_DESCRIPTOR_SETS
#ifdef TEST_DESCRIPTOR_SETS
	{
		std::array<std::uint32_t, 1> white_tex_data = { 1 };
		DataBuffer pixels { white_tex_data.data(), sizeof(std::uint32_t) };
		TextureProperties white_tex_props {
			.extent = extent,
			.format = ImageFormat::SBGR,
			.debug_name = "white_tex",
		};
		Ref<Texture> white_tex = Texture::construct(device, white_tex_props);
		renderer.expose_to_shaders(*white_tex);
	}

#endif
}

Scene::~Scene()
{
	SceneSerialiser scene_serialiser(*this);
	registry.clear();
}

void Scene::update(float)
{
	if (picked_entity->is_valid()) {
		selected_entity.swap(picked_entity);
		picked_entity = make_scope<Entity>(*this);
	}
}

void Scene::render(Renderer& renderer)
{
	command_executor->begin();
	{
		renderer.begin_pass(*command_executor, *framebuffer, true);

		auto mesh_view = registry.view<const Components::Mesh, const Components::Pipeline, const Components::Transform>();
		for (auto&& [entity, mesh, pipeline, transform] : mesh_view.each()) {
			renderer.draw_mesh(*command_executor, *mesh.mesh, *pipeline.pipeline, transform.compute());
		}

		renderer.end_pass(*command_executor);
	}
	{
		renderer.begin_pass(*command_executor, *framebuffer);

		auto line_view = registry.view<const Components::LineGeometry, const Components::Texture, const Components::Transform>();
		for (auto&& [entity, geom, tex, transform] : line_view.each()) {
			renderer.draw_planar_geometry(Geometry::Line,
				{
					.position = transform.position,
					.to_position = geom.to_position,
					.colour = tex.colour,
				});
		}

		renderer.end_pass(*command_executor);
	}
	{
		renderer.begin_pass(*command_executor, *identity_framebuffer);

		auto rect_view
			= registry.view<const Components::Texture, const Components::QuadGeometry, const Components::Transform, const Components::ID>();
		for (auto&& [entity, tex, geom, transform, id] : rect_view.each()) {
			renderer.draw_planar_geometry(Geometry::Rectangle,
				{
					.position = transform.position,
					.colour = tex.colour,
					.rotation = transform.rotation,
					.dimensions = { transform.scale },
					.identifier = { id.get_id<std::uint32_t>() },
				});
		}

		// TODO: Implement
		// renderer.draw_text("Hello world!", 0, 0, 12.f);

		renderer.end_pass(*command_executor);
	}
	command_executor->submit_and_end();
}

void Scene::on_event(Event& event) { }

void Scene::recreate(const Extent& new_ex)
{
	extent = new_ex;

	identity_framebuffer->recreate(true, extent);
	framebuffer->recreate(true, extent);
	command_executor->recreate(true, extent);
}

void Scene::destruct() { command_executor.reset(); }

Entity Scene::create(std::string_view name)
{
	auto entity = Entity(*this, name);
	return entity;
}

void Scene::delete_entity(entt::entity entity) { registry.destroy(entity); }

void Scene::delete_entity(const Entity& entity) { delete_entity(entity.get_identifier()); }

Scope<Scene> Scene::deserialise(const Device& device, std::string_view name, const std::filesystem::path& filename)
{
	Scope<Scene> created = make_scope<Scene>(device, name);
	SceneDeserialiser deserialiser { *created, device, filename };
	return created;
}

void Scene::update_picked_entity(std::uint32_t handle) { picked_entity = make_scope<Entity>(*this, handle == 0 ? entt::null : handle); }

void Scene::manipulate_entity_transform(Entity& entity, Camera& camera, GizmoType gizmo_type)
{
	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

	const auto& camera_view = camera.get_view_matrix();
	const auto& camera_projection = camera.get_projection_matrix();
	auto copy = camera_projection;
	copy[1][1] *= -1.0f;

	auto& entity_transform = entity.get_components<Components::Transform>();
	auto transform = entity_transform.compute();

	bool snap = Input::key_pressed(KeyCode::LeftShift);
	float snap_value = 0.5f;
	if (static_cast<ImGuizmo::OPERATION>(gizmo_type) == ImGuizmo::OPERATION::ROTATE) {
		snap_value = 45.0f;
	}

	std::array snap_values = { snap_value, snap_value, snap_value };

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

std::optional<Entity> Scene::get_by_identifier(Identifier identifier)
{
	for (const auto [entity, id] : registry.view<Components::ID>().each()) {
		if (id.identifier == identifier)
			return Entity { *this, entity };
	}

	return std::nullopt;
}

} // namespace Disarray
