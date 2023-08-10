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
#include "graphics/Texture.hpp"
#include "scene/Components.hpp"
#include "scene/Entity.hpp"

#include <entt/entt.hpp>

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
		auto parent = create("Grid");
		for (auto j = -rects_y / 2; j < rects_y / 2; j++) {
			for (auto i = -rects_x / 2; i < rects_x / 2; i++) {
				auto rect = create(fmt::format("Rect{}-{}", i, j));
				parent.add_child(&rect);
				auto& transform = rect.get_components<Transform>();
				transform.position = { 2 * static_cast<float>(i) + 0.5f, -1, 2 * static_cast<float>(j) + 0.5f };
				transform.rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3 { 1, 0, 0 });
				glm::vec4 colour {};
				rect.add_component<Components::Geometry>(Geometry::Rectangle);
				rect.add_component<QuadGeometry>();
				rect.add_component<Components::Texture>();
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

		for (auto i = 0; i < 4; i++) {
			auto rect = create(fmt::format("Rect{}", i));
			auto& transform = rect.get_components<Transform>();
			transform.position = { 2 * static_cast<float>(i) + 0.5f, -1, 0 };
			transform.rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3 { 1, 0, 0 });
			glm::vec4 colour { i / 3.f, 0.3, i / 3.f, 1.f };
			rect.add_component<Components::Geometry>(Geometry::Rectangle);
			rect.add_component<QuadGeometry>();
			rect.add_component<Components::Texture>(colour);
		}

		auto unit_vectors = create("UnitVectors");
		{
			const glm::vec3 base_pos { 0, 0, 0 };
			{
				auto axis = create("XAxis");
				auto& transform = axis.get_components<Transform>();
				transform.position = base_pos;
				axis.add_component<LineGeometry>(base_pos + glm::vec3 { 10.0, 0, 0 });
				axis.add_component<Components::Texture>(glm::vec4 { 1, 0, 0, 1 });
				unit_vectors.add_child(axis);
			}
			{
				auto axis = create("YAxis");
				auto& transform = axis.get_components<Transform>();
				transform.position = base_pos;
				axis.add_component<LineGeometry>(base_pos + glm::vec3 { 0, -10.0, 0 });
				axis.add_component<Components::Texture>(glm::vec4 { 0, 1, 0, 1 });
				unit_vectors.add_child(axis);
			}
			{
				auto axis = create("ZAxis");
				auto& transform = axis.get_components<Transform>();
				transform.position = base_pos;
				axis.add_component<LineGeometry>(base_pos + glm::vec3 { 0, 0, -10.0 });
				axis.add_component<Components::Texture>(glm::vec4 { 0, 0, 1, 1 });
				unit_vectors.add_child(axis);
			}
		}

		command_executor = CommandExecutor::construct(device, swapchain, { .count = 3, .is_primary = true, .record_stats = true });
		renderer.on_batch_full([&exec = *command_executor](Renderer& r) { r.flush_batch(exec); });

		VertexLayout layout {
			{ ElementType::Float3, "position" },
			{ ElementType::Float2, "uv" },
			{ ElementType::Float4, "colour" },
			{ ElementType::Float3, "normals" },
		};

		auto extent = swapchain.get_extent();

		framebuffer = Framebuffer::construct(device,
			{
				.extent = swapchain.get_extent(),
				.attachments = { { Disarray::ImageFormat::SBGR }, { ImageFormat::Depth } },
				.clear_colour_on_load = false,
				.clear_depth_on_load = false,
				.debug_name = "FirstFramebuffer",
			});

		identity_framebuffer = Framebuffer::construct(device,
			{
				.extent = swapchain.get_extent(),
				.attachments = { { ImageFormat::SBGR }, { ImageFormat::Uint, false }, { ImageFormat::Depth } },
				.clear_colour_on_load = true,
				.clear_depth_on_load = true,
				.debug_name = "IdentityFramebuffer",
			});

		const auto& [vert, frag] = renderer.get_pipeline_cache().get_shader("main");
		PipelineProperties props = {
			.vertex_shader = vert,
			.fragment_shader = frag,
			.framebuffer = framebuffer,
			.layout = layout,
			.push_constant_layout = PushConstantLayout { PushConstantRange { PushConstantKind::Both, std::size_t { 84 } } },
			.extent = extent,
			.samples = SampleCount::ONE,
			.depth_comparison_operator = DepthCompareOperator::GreaterOrEqual,
			.cull_mode = CullMode::Back,
			.descriptor_set_layout = renderer.get_descriptor_set_layouts().data(),
			.descriptor_set_layout_count = static_cast<std::uint32_t>(renderer.get_descriptor_set_layouts().size()),
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

	void Scene::update(float) { }

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

			auto line_view = registry.view<const LineGeometry, const Components::Texture, const Transform>();
			for (const auto& [entity, geom, tex, transform] : line_view.each()) {
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

			auto rect_view = registry.view<const Components::Texture, const QuadGeometry, const Transform, const ID>();
			for (const auto& [entity, tex, geom, transform, id] : rect_view.each()) {
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

	void Scene::on_event(Event& event)
	{
		EventDispatcher dispatcher(event);

		dispatcher.dispatch<KeyPressedEvent>([&max = vp_max, &min = vp_min, &fb = identity_framebuffer](KeyPressedEvent& pressed) {
			if (pressed.get_key_code() == KeyCode::L) {
				const auto& image = fb->get_image(1);
				auto pos = Input::mouse_position();
				pos -= min;

				pos.x /= (max.x - min.x);
				pos.y /= max.y;
				glm::vec4 pixel_data = image.read_pixel(pos);
				Log::debug("Scene - PixelData", "{}", pixel_data);
			}
			return true;
		});
	}

	void Scene::recreate(const Extent& extent)
	{
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

} // namespace Disarray
