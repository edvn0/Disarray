#include "ClientLayer.hpp"

namespace Disarray::Client {
	AppLayer::AppLayer(Device& dev, Window& win, Swapchain& swap)
		: device(dev)
		, window(win)
		, swapchain(swap) {

		};

	AppLayer::~AppLayer() {

	}

	void AppLayer::interface()
	{
		ImGui::Begin("App");
		auto& img = framebuffer->get_image(0);
		UI::image_button(img);
		ImGui::End();
	}

	void AppLayer::construct(App& app, Renderer& renderer)
	{
		VertexLayout layout { {
			{ ElementType::Float3, "position" },
			{ ElementType::Float2, "uv" },
			{ ElementType::Float4, "colour" },
			{ ElementType::Float3, "normals" },
		} };

		auto extent = swapchain.get_extent();

		framebuffer = Framebuffer::construct(device, swapchain,
			{
				.format = Disarray::ImageFormat::SBGR,
				.load_colour = false,
				.keep_colour = true,
				.load_depth = false,
				.keep_depth = true,
				.has_depth = true,
				.debug_name = "FirstFramebuffer"
			});
		const auto& [vert, frag] = renderer.get_pipeline_cache().get_shader("main");
		PipelineProperties props = {
			.vertex_shader = vert,
			.fragment_shader = frag,
			.framebuffer = framebuffer,
			.layout = layout,
			.push_constant_layout = PushConstantLayout { PushConstantRange { PushConstantKind::Both, std::size_t { 80 } } },
			.extent = { extent.width, extent.height },
		};
		pipeline = Pipeline::construct(device, swapchain, props);

		second_framebuffer = Framebuffer::construct(device, swapchain,
			{
				.format = Disarray::ImageFormat::SBGR,
				.load_colour = true,
				.keep_colour = true,
				.load_depth = true,
				.keep_depth = true,
				.has_depth = true,
				.debug_name = "SecondFramebuffer"
			});

		command_executor = CommandExecutor::construct(device, swapchain, { .count = 3, .is_primary = true });

		viking_mesh = Mesh::construct(device, swapchain, { .path = "Assets/Models/viking.mesh", .pipeline = pipeline });
#define IS_TESTING
#ifdef IS_TESTING
		{
			std::uint32_t white_tex[] = { 0 };
			auto pixels = DataBuffer(white_tex, sizeof(std::uint32_t));
			TextureProperties texture_properties { .extent = swapchain.get_extent(), .format = ImageFormat::SBGR, .debug_name = "white_tex" };
			Ref<Texture> tex = Texture::construct(device, swapchain, texture_properties);
			texture_properties.path = "Assets/Textures/viking_room.png";
			texture_properties.debug_name = "viking";
			viking_room = Texture::construct(device, swapchain, texture_properties);
		};

#endif
#undef IS_TESTING
	};

	void AppLayer::handle_swapchain_recreation(Renderer& renderer)
	{
		renderer.set_extent(swapchain.get_extent());
		pipeline->force_recreation();
		viking_room->force_recreation();
		framebuffer->force_recreation();
		second_framebuffer->force_recreation();
	}

	void AppLayer::update(float ts) {

	};

	void AppLayer::update(float ts, Renderer& renderer)
	{
		command_executor->begin();
		{
			renderer.begin_pass(*command_executor, *framebuffer);
			// const auto&& [mid_x, mid_y] = renderer.center_position();
			renderer.draw_mesh(*command_executor, *viking_mesh);

			renderer.end_pass(*command_executor);
		}
		{
			renderer.begin_pass(*command_executor, *second_framebuffer);
			// const auto&& [mid_x, mid_y] = renderer.center_position();
			static glm::vec3 pos { 0, 0, 0 };
			renderer.draw_planar_geometry(Geometry::Rectangle, { .position = pos, .dimensions = { { 1.f, 1.f, 1.f } } });
			pos += 0.001;
			// renderer.draw_text("Hello world!", 0, 0, 12.f);
			// static glm::vec3 pos_circle {-0.5,0,0};
			renderer.draw_planar_geometry(Geometry::Line, { .position = { -0.5, -0.5, 0 }, .to_position = { 0.5, 0.5, 0 } });
			// renderer.draw_planar_geometry(Geometry::Circle, { .position = pos_circle, .dimensions = { { 1.f, 1.f, 1.f } } });

			renderer.submit_batched_geometry(*command_executor);
			renderer.end_pass(*command_executor);
		}

		command_executor->submit_and_end();
	}

	void AppLayer::destruct() { }

}