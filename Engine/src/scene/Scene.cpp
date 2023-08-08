#include "DisarrayPCH.hpp"

#include "scene/Scene.hpp"

#include "core/App.hpp"
#include "core/Formatters.hpp"
#include "core/Input.hpp"
#include "core/ThreadPool.hpp"
#include "core/events/Event.hpp"
#include "core/events/KeyEvent.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Renderer.hpp"
#include "scene/Components.hpp"
#include "scene/Entity.hpp"

#include <entt/entt.hpp>

template <class Position = glm::vec3> static constexpr auto draw_axes(auto& renderer, const Position& position)
{
	// X is red
	renderer.draw_planar_geometry(Disarray::Geometry::Line,
		Disarray::GeometryProperties { .position = position, .to_position = position + glm::vec3 { 10.0, 0, 0 }, .colour = { 1, 0, 0, 1 } });

	// Y is green
	renderer.draw_planar_geometry(Disarray::Geometry::Line,
		Disarray::GeometryProperties { .position = position, .to_position = position + glm::vec3 { 0, -10.0, 0 }, .colour = { 0, 1, 0, 1 } });

	// Z is blue
	renderer.draw_planar_geometry(Disarray::Geometry::Line,
		Disarray::GeometryProperties { .position = position, .to_position = position + glm::vec3 { 0, 0, -10.0 }, .colour = { 0, 0, 1, 1 } });
}

static auto rotate_by(const glm::vec3& axis_radians)
{
	glm::mat4 identity = glm::identity<glm::mat4>();
	glm::rotate(identity, axis_radians.x, glm::vec3 { 1, 0, 0 });
	glm::rotate(identity, axis_radians.y, glm::vec3 { 0, 1, 0 });
	glm::rotate(identity, axis_radians.z, glm::vec3 { 0, 0, 1 });
	return identity;
}

namespace Disarray {

	Scene::Scene(Device& dev, Window& win, Swapchain& sc, std::string_view name)
		: device(dev)
		, window(win)
		, swapchain(sc)
		, scene_name(name)
		, registry(entt::basic_registry())
	{
		(void)window.native();
		int rects_x { 6 };
		int rects_y { 6 };
		auto parent = create("Grid");
		for (auto i = -3; i < rects_x + 1; i++) {
			for (auto j = -3; j < rects_y + 1; j++) {
				auto rect = create(fmt::format("Rect{}", i));
				parent.add_child(&rect);
				auto& transform = rect.get_components<Transform>();
				transform.position = { static_cast<float>(i), 0, static_cast<float>(j) };
			}
		}
	}

	void Scene::construct(Disarray::App&, Disarray::Renderer& renderer, Disarray::ThreadPool&)
	{
		// TODO: Record stats does not work with recreation of query pools.
		command_executor = CommandExecutor::construct(device, swapchain, { .count = 3, .is_primary = true, .record_stats = true });
		renderer.on_batch_full([&exec = command_executor](Renderer& r) { r.flush_batch(*exec); });

		VertexLayout layout { {
			{ ElementType::Float3, "position" },
			{ ElementType::Float2, "uv" },
			{ ElementType::Float4, "colour" },
			{ ElementType::Float3, "normals" },
		} };

		auto extent = swapchain.get_extent();

		framebuffer = Framebuffer::construct(device,
			{ .format = Disarray::ImageFormat::SBGR,
				.samples = SampleCount::ONE,
				.extent = swapchain.get_extent(),
				.load_colour = true,
				.keep_colour = true,
				.load_depth = true,
				.keep_depth = true,
				.has_depth = true,
				.should_present = false,
				.debug_name = "FirstFramebuffer" });

		const auto& [vert, frag] = renderer.get_pipeline_cache().get_shader("main");
		PipelineProperties props = {
			.vertex_shader = vert,
			.fragment_shader = frag,
			.framebuffer = framebuffer,
			.layout = layout,
			.push_constant_layout = PushConstantLayout { PushConstantRange { PushConstantKind::Both, std::size_t { 84 } } },
			.extent = { extent.width, extent.height },
			.samples = SampleCount::ONE,
			.descriptor_set_layout = renderer.get_descriptor_set_layouts().data(),
			.descriptor_set_layout_count = static_cast<std::uint32_t>(renderer.get_descriptor_set_layouts().size()),
		};
		auto viking_rotation = rotate_by(glm::radians(glm::vec3 { 180, 180, 180 }));

		auto v_mesh = create("Viking");
		v_mesh.add_component<Components::Mesh>(Mesh::construct(device, { .path = "Assets/Models/viking.mesh", .initial_rotation = viking_rotation }));
		v_mesh.add_component<Components::Pipeline>(Pipeline::construct(device, props));

#define TEST_DESCRIPTOR_SETS
#ifdef TEST_DESCRIPTOR_SETS
		{
			std::array<std::uint32_t, 1> white_tex_data = { 1 };
			DataBuffer pixels { white_tex_data.data(), sizeof(std::uint32_t) };
			TextureProperties white_tex_props { .extent = swapchain.get_extent(), .format = ImageFormat::SBGR, .debug_name = "white_tex" };
			Ref<Texture> white_tex = Texture::construct(device, white_tex_props);
			renderer.expose_to_shaders(*white_tex);
		}
#endif

		TextureProperties texture_properties { .extent = swapchain.get_extent(), .format = ImageFormat::SBGR, .debug_name = "viking" };
		texture_properties.path = "Assets/Textures/viking_room.png";
		v_mesh.add_component<Components::Texture>(Texture::construct(device, texture_properties));
	}

	Scene::~Scene() { registry.clear(); }

	void Scene::update(float ts)
	{
		static glm::vec3 viking_pos { 1.f };
		static float viking_rot { 180.f };
		const glm::quat viking_rotation = glm::angleAxis(glm::radians(viking_rot), glm::vec3 { 0, 1, 0 });
		static glm::vec3 viking_scale { 2.5f };
		viking_rot += 0.01 * ts;
		viking_pos.x += 0.01 * ts;
		viking_pos.z += 0.01 * ts;

		auto mesh_view = registry.view<const Components::Mesh, Transform>();
		for (auto [entity, _, transform] : mesh_view.each()) {
			transform.position = viking_pos;
			transform.rotation = viking_rotation;
			transform.scale = viking_scale;
		}
	}

	void Scene::render(Renderer& renderer)
	{
		command_executor->begin();
		{
			renderer.begin_pass(*command_executor, *framebuffer, true);
			auto mesh_view = registry.view<const Components::Mesh, const Components::Pipeline, const Transform>();
			for (const auto& [entity, mesh, pipeline, transform] : mesh_view.each()) {
				renderer.draw_mesh(*command_executor, *mesh.mesh, *pipeline.pipeline, transform.compute());
			}

			renderer.end_pass(*command_executor);
		}
		{
			renderer.begin_pass(*command_executor, *framebuffer);

			auto rect_view = registry.view<const Transform, const ID>();
			for (const auto& [entity, transform, id] : rect_view.each()) {
				renderer.draw_planar_geometry(Geometry::Rectangle,
					{ .position = transform.position,
						.rotation = transform.rotation,
						.identifier = { id.get_identifier() },
						.dimensions = { transform.scale } });
			}

			// TODO: Implement
			// renderer.draw_text("Hello world!", 0, 0, 12.f);

			// TODO: Temporary
			draw_axes(renderer, glm::vec3 { 0, 0, 0 });

			renderer.submit_batched_geometry(*command_executor);
			renderer.end_pass(*command_executor);
		}
		command_executor->submit_and_end();
	}

	void Scene::on_event(Event& event)
	{
		EventDispatcher dispatcher(event);

		dispatcher.dispatch<KeyPressedEvent>([&reg = registry](KeyPressedEvent& pressed) {
			if (pressed.get_key_code() != KeyCode::K)
				return false;
			auto view = reg.view<Inheritance>();
			for (const auto& [entity, inheritance] : view.each()) {
				const auto& [children, parent] = inheritance;
				Log::debug("Inheritance info", "parent: {}, children: {}", parent, fmt::join(children, ", "));
			}
			return true;
		});
	}

	void Scene::recreate(const Extent& extent)
	{
		framebuffer->recreate(true, extent);
		command_executor->recreate(true, extent);
	}

	Framebuffer& Scene::get_framebuffer() { return *framebuffer; }

	void Scene::destruct() { command_executor.reset(); }

	Entity Scene::create(std::string_view name)
	{
		// TODO: ID will need some UUID, for now, lets just increase by one
		static Identifier identifier { 0 };

		auto handle = registry.create();
		auto entity = Entity(*this, handle, name);
		entity.add_component<Transform>();
		entity.add_component<ID>(identifier++);
		return entity;
	}

} // namespace Disarray
