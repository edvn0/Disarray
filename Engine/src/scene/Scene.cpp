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
	}

	void Scene::construct(Disarray::App&, Disarray::Renderer& renderer, Disarray::ThreadPool&)
	{
		int rects_x { 2 };
		int rects_y { 2 };
		std::size_t loops { 0 };
		auto parent = create("Grid");
		for (auto j = -1; j < rects_y; j++) {
			for (auto i = -1; i < rects_x; i++) {
				auto rect = create(fmt::format("Rect{}", i));
				parent.add_child(&rect);
				auto& transform = rect.get_components<Transform>();
				transform.position = { static_cast<float>(i) + 0.5f, -1, static_cast<float>(j) + 0.5f };
				transform.rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3 { 1, 0, 0 });
				rect.add_component<Components::Geometry>(Geometry::Rectangle);
				rect.add_component<Components::Texture>();
				loops++;
			}
		}

		auto floor = create("Floor");
		floor.add_component<Components::Geometry>();
		floor.add_component<Components::Texture>(glm::vec4 { 0.2, 0.2, 0.8, 1.0f });
		auto& floor_transform = floor.get_components<Transform>();
		floor_transform.scale = { 100, 100, 1 };
		floor_transform.rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3 { 1, 0, 0 });

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
			{ .extent = swapchain.get_extent(),
				.attachments = { { Disarray::ImageFormat::SBGR }, { ImageFormat::Depth } },
				.clear_colour_on_load = false,
				.clear_depth_on_load = false,
				.debug_name = "FirstFramebuffer" });

		identity_framebuffer = Framebuffer::construct(device,
			{ .extent = swapchain.get_extent(),
				.attachments = { { ImageFormat::SBGR },  { ImageFormat::Uint, false },{ ImageFormat::Depth }, },
				.clear_colour_on_load = true,
				.clear_depth_on_load = true,
				.debug_name = "IdentityFramebuffer" });

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
		static glm::vec3 viking_scale { 2.5f };
		viking_pos.x += 0.001 * ts;
		viking_pos.z += 0.001 * ts;

		auto mesh_view = registry.view<const Components::Mesh, Transform>();
		for (auto [entity, _, transform] : mesh_view.each()) {
			transform.position = viking_pos;
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

			// TODO: Temporary
			draw_axes(renderer, glm::vec3 { 0, 0, 0 });
			renderer.submit_batched_geometry(*command_executor);
			renderer.end_pass(*command_executor);
		}
		{
			renderer.begin_pass(*command_executor, *identity_framebuffer);

			auto rect_view = registry.view<const Components::Texture, const Components::Geometry, const Transform, const ID>();
			for (const auto& [entity, tex, geom, transform, id] : rect_view.each()) {
				renderer.draw_planar_geometry(geom.geometry,
					{ .position = transform.position,
						.colour = tex.colour,
						.rotation = transform.rotation,
						.identifier = { id.get_identifier() },
						.dimensions = { transform.scale } });
			}

			// TODO: Implement
			// renderer.draw_text("Hello world!", 0, 0, 12.f);

			renderer.submit_batched_geometry(*command_executor);
			renderer.end_pass(*command_executor);
		}
		command_executor->submit_and_end();
	}

	void Scene::on_event(Event& event)
	{
		EventDispatcher dispatcher(event);

		dispatcher.dispatch<KeyPressedEvent>([&max = vp_max, &min = vp_min, &reg = registry, &fb = identity_framebuffer](KeyPressedEvent& pressed) {
			if (pressed.get_key_code() == KeyCode::L) {
				const auto& image = fb->get_image(1);
				auto pos = Input::mouse_position();
				pos -= min;

				pos.x /= (max.x - min.x);
				pos.y /= max.y;
				glm::vec4 pixel_data = image.read_pixel(pos);
				Log::debug("Scene - PixelData", fmt::format("{}", pixel_data));
			}

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
