#include "DisarrayPCH.hpp"

#include "scene/Scene.hpp"

#include "core/App.hpp"
#include "core/ThreadPool.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Renderer.hpp"
#include "scene/Entity.hpp"

#include <entt/entt.hpp>

template <class Position = glm::vec3> static constexpr auto draw_axes(auto& renderer, const Position& position)
{
	renderer.draw_planar_geometry(Disarray::Geometry::Line,
		Disarray::GeometryProperties { .position = position, .to_position = position + glm::vec3 { 10.0, 0, 0 }, .colour = { 1, 0, 0, 1 } });
	renderer.draw_planar_geometry(Disarray::Geometry::Line,
		Disarray::GeometryProperties { .position = position, .to_position = position + glm::vec3 { 0, -10.0, 0 }, .colour = { 0, 1, 0, 1 } });
	renderer.draw_planar_geometry(Disarray::Geometry::Line,
		Disarray::GeometryProperties { .position = position, .to_position = position + glm::vec3 { 0, 0, -10.0 }, .colour = { 0, 0, 1, 1 } });
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
		std::size_t rects { 10 };
		auto j = 0;
		for (std::size_t i = 0; i < rects; i++) {
			auto rect = create(fmt::format("Rect{}", i));
			auto& transform = rect.get_components<Transform>();
			transform.position = { i % 3, i % 3, j++ };
		}
	}

	void Scene::construct(Disarray::App&, Disarray::Renderer& renderer, Disarray::ThreadPool&)
	{
		// TODO: Record stats does not work with recreation of query pools.
		command_executor = CommandExecutor::construct(device, swapchain, { .count = 3, .is_primary = true, .record_stats = true });

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
		pipeline = Pipeline::construct(device, props);

		auto viking_rotation = glm::rotate(glm::mat4 { 1.0f }, glm::radians(90.0f), glm::vec3(1, 0, 0))
			* glm::rotate(glm::mat4 { 1.0f }, glm::radians(90.0f), glm::vec3(0, 1, 0));
		viking_mesh
			= Mesh::construct(device, swapchain, { .path = "Assets/Models/viking.mesh", .pipeline = pipeline, .initial_rotation = viking_rotation });
#define IS_TESTING
#ifdef IS_TESTING
		{
			std::array<std::uint32_t, 1> white_tex_data = { 1 };
			DataBuffer pixels { white_tex_data.data(), sizeof(std::uint32_t) };
			TextureProperties texture_properties { .extent = swapchain.get_extent(), .format = ImageFormat::SBGR, .debug_name = "white_tex" };
			Ref<Texture> white_tex = Texture::construct(device, texture_properties);
			renderer.expose_to_shaders(*white_tex);

			texture_properties.path = "Assets/Textures/viking_room.png";
			texture_properties.debug_name = "viking";
			viking_room = Texture::construct(device, texture_properties);
		};

#endif
#undef IS_TESTING
	}

	Scene::~Scene() { registry.clear(); }

	void Scene::update(float) { }

	void Scene::render(float ts, Renderer& renderer)
	{
		command_executor->begin();
		{
			renderer.begin_pass(*command_executor, *framebuffer, true);
			static glm::vec3 pos { -1.f };
			static float rot { 180.f };
			const glm::quat rotation = glm::angleAxis(glm::radians(rot), glm::vec3 { 0, 1, 0 });
			static glm::vec3 scale { 2.5f };
			rot += 0.01 * ts;
			renderer.draw_mesh(*command_executor, *viking_mesh, { .position = pos, .scale = scale, .rotation = rotation });

			renderer.end_pass(*command_executor);
		}
		{
			renderer.begin_pass(*command_executor, *framebuffer);

			auto rect_view = registry.view<const Transform, const ID>();
			for (const auto [entity, transform, id] : rect_view.each()) {
				auto ide = id.get_identifier();
				renderer.draw_planar_geometry(Geometry::Rectangle,
					{ .position = transform.position, .rotation = transform.rotation, .identifier = { ide }, .dimensions = { transform.scale } });
			}
			// renderer.draw_text("Hello world!", 0, 0, 12.f);
			draw_axes(renderer, glm::vec3 { 0, -0.1, 0 });

			renderer.submit_batched_geometry(*command_executor);
			renderer.end_pass(*command_executor);
		}
		command_executor->submit_and_end();
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
