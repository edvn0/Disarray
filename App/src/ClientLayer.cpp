#include "ClientLayer.hpp"

#include "glm/ext/matrix_transform.hpp"
#include "panels/ExecutionStatisticsPanel.hpp"

#include <Disarray.hpp>
#include <array>

namespace Disarray::Client {

	template <class Position = glm::vec3> static constexpr auto axes(Renderer& renderer, const Position& position)
	{
		renderer.draw_planar_geometry(Geometry::Line, { .position = position, .to_position = position + glm::vec3 { 10.0, 0, 0 } });
		renderer.draw_planar_geometry(Geometry::Line, { .position = position, .to_position = position + glm::vec3 { 0, -10.0, 0 } });
		renderer.draw_planar_geometry(Geometry::Line, { .position = position, .to_position = position + glm::vec3 { 0, 0, -10.0 } });
	}

	AppLayer::AppLayer(Device& dev, Window& win, Swapchain& swap)
		: device(dev)
		, window(win)
		, swapchain(swap) {};

	AppLayer::~AppLayer() = default;

	class B : public Panel {
	public:
		B(Device& dev, Window& win, Swapchain& swap)
			: device(dev)
			, window(win)
			, swapchain(swap)
		{
			device.get_physical_device();
			window.get_framebuffer_scale();
			swapchain.get_current_frame();
			begin = Clock::ms();
		};

		void interface() override
		{
			ImGui::Begin("B Timestep");
			auto ts = Clock::ms() - begin;
			ImGui::TextColored({ 1.f, 1.f, 1.0f, 1.0f }, "Timestep (panel B) between panel draws: %Fms", ts);
			ImGui::End();
			begin = Clock::ms();
		}

	private:
		float begin { 0.0f };

		Device& device;
		Window& window;
		Swapchain& swapchain;
	};

	class C : public Panel {
	public:
		C(Device& dev, Window& win, Swapchain& swap)
			: device(dev)
			, window(win)
			, swapchain(swap)
		{
			device.get_physical_device();
			window.get_framebuffer_scale();
			swapchain.get_current_frame();
			begin = Clock::ms();
		};

		void interface() override
		{
			ImGui::Begin("C Timestep");
			auto ts = Clock::ms() - begin;
			ImGui::TextColored({ 1.f, 1.f, 1.0f, 1.0f }, "Timestep (panel C) between panel draws: %Fms", ts);
			ImGui::End();
			begin = Clock::ms();
		}

	private:
		float begin { 0.0f };

		Device& device;
		Window& window;
		Swapchain& swapchain;
	};

	void AppLayer::construct(App& app, Renderer& renderer, ThreadPool& pool)
	{
		app.add_panel<StatisticsPanel>(app.get_statistics());
		app.add_panel<B>();
		app.add_panel<C>();
		// TODO: Record stats does not work with recreation of query pools.
		command_executor = CommandExecutor::construct(device, swapchain, { .count = 3, .is_primary = true, .record_stats = true });
		app.add_panel<ExecutionStatisticsPanel>(*command_executor);

		VertexLayout layout { {
			{ ElementType::Float3, "position" },
			{ ElementType::Float2, "uv" },
			{ ElementType::Float4, "colour" },
			{ ElementType::Float3, "normals" },
		} };

		auto extent = swapchain.get_extent();

		framebuffer = Framebuffer::construct(device, swapchain,
			{ .format = Disarray::ImageFormat::SBGR,
				.samples = SampleCount::ONE,
				.load_colour = true,
				.keep_colour = true,
				.load_depth = true,
				.keep_depth = true,
				.has_depth = true,
				.should_present = false,
				.debug_name = "FirstFramebuffer" });

		const auto& [vert, frag] = renderer.get_pipeline_cache().get_shader("main");
		std::array<VkDescriptorSetLayout, 1> layouts { renderer.get_descriptor_set_layout() };
		PipelineProperties props = {
			.vertex_shader = vert,
			.fragment_shader = frag,
			.framebuffer = framebuffer,
			.layout = layout,
			.push_constant_layout = PushConstantLayout { PushConstantRange { PushConstantKind::Both, std::size_t { 80 } } },
			.extent = { extent.width, extent.height },
			.samples = SampleCount::ONE,
			.descriptor_set_layout = layouts.data(),
			.descriptor_set_layout_count = renderer.get_descriptor_set_layout_count(),
		};
		pipeline = Pipeline::construct(device, swapchain, props);

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
			Ref<Texture> white_tex = Texture::construct(device, swapchain, texture_properties);
			renderer.expose_to_shaders(*white_tex);

			texture_properties.path = "Assets/Textures/viking_room.png";
			texture_properties.debug_name = "viking";
			viking_room = Texture::construct(device, swapchain, texture_properties);
		};

#endif
#undef IS_TESTING
	};

	void AppLayer::interface()
	{
		// ImGui + Dockspace Setup ------------------------------------------------------------------------------
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();

		io.ConfigWindowsResizeFromEdges = io.BackendFlags & ImGuiBackendFlags_HasMouseCursors;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		bool is_maximized = UI::is_maximised(window);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, is_maximized ? ImVec2(6.0f, 6.0f) : ImVec2(1.0f, 1.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);
		ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4 { 0.0f, 0.0f, 0.0f, 0.0f });
		ImGui::Begin("Dockspace", nullptr, window_flags);
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(2);

		ImGui::PopStyleVar(2);

		// Dockspace
		float min_win_size_x = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		ImGui::DockSpace(ImGui::GetID("Dockspace"));
		style.WindowMinSize.x = min_win_size_x;

		// Editor Panel ------------------------------------------------------------------------------
		// ImGui::ShowDemoWindow();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Viewport");
		{
			viewport_panel_mouse_over = ImGui::IsWindowHovered();
			viewport_panel_focused = ImGui::IsWindowFocused();

			auto viewport_offset = ImGui::GetCursorPos(); // includes tab bar
			auto viewport_size = ImGui::GetContentRegionAvail();
			// client_scene->resize({ viewport_size.x, viewport_size.y });
			// editor_camera.resize({ viewport_size.x, viewport_size.y });

			// Render viewport image
			auto& image = framebuffer->get_image();
			UI::image(image, { viewport_size.x, viewport_size.y });

			auto window_size = ImGui::GetWindowSize();
			ImVec2 min_bound = ImGui::GetWindowPos();

			// NOTE(Peter): This currently just subtracts 0 because I removed the toolbar window, but I'll keep it in just in case
			min_bound.x -= viewport_offset.x;
			min_bound.y -= viewport_offset.y;

			ImVec2 max_bound = { min_bound.x + window_size.x, min_bound.y + window_size.y };
			viewport_bounds[0] = { min_bound.x, min_bound.y };
			viewport_bounds[1] = { max_bound.x, max_bound.y };
		}
		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::End();
	}

	struct Controller {
		glm::vec3& pos;
		glm::mat4& rot;

		void update(float ts)
		{
			if (Input::button_pressed(KeyCode::A)) {
				pos.x -= ts * 0.001f;
			}

			if (Input::button_pressed(KeyCode::D)) {
				pos.x += ts * 0.001f;
			}

			if (Input::button_pressed(KeyCode::W)) {
				pos.y -= ts * 0.001f;
			}

			if (Input::button_pressed(KeyCode::S)) {
				pos.y += ts * 0.001f;
			}

			if (Input::button_pressed(KeyCode::Q)) {
				rot = glm::rotate(rot, ts * glm::radians(1.f), glm::vec3 { 0, 0, 1 });
			}

			if (Input::button_pressed(KeyCode::E)) {
				rot = glm::rotate(rot, ts * -glm::radians(1.f), glm::vec3 { 0, 0, 1 });
			}
		}
	};

	void AppLayer::handle_swapchain_recreation(Renderer& renderer)
	{
		pipeline->recreate(true);
		viking_room->recreate(true);
		framebuffer->recreate(true);
		command_executor->recreate(true);
	}

	void AppLayer::update(float ts) {

	};

	void AppLayer::update(float ts, Renderer& renderer)
	{
		command_executor->begin();
		{
			renderer.begin_pass(*command_executor, *framebuffer, true);
			static glm::vec3 pos { -1.f };
			static glm::quat rotation = glm::angleAxis(glm::radians(45.f), glm::vec3 { 0, 1, 0 });
			static glm::vec3 scale { 0.5f };
			renderer.draw_mesh(*command_executor, *viking_mesh, { .position = pos, .scale = scale, .rotation = rotation });

			renderer.end_pass(*command_executor);
		}
		{
			renderer.begin_pass(*command_executor, *framebuffer);
			static glm::vec3 pos { 0, 0, 0 };
			static glm::mat4 rot { 1.0f };
			static Controller controller { pos, rot };
			controller.update(ts);
			renderer.draw_planar_geometry(Geometry::Rectangle, { .position = pos, .rotation = rot, .dimensions = { { 1.f, 1.f, 1.f } } });
			// renderer.draw_text("Hello world!", 0, 0, 12.f);
			// static glm::vec3 pos_circle {-0.5,0,0};
			axes(renderer, glm::vec3 { 0, -0.1, 0 });
			// renderer.draw_planar_geometry(Geometry::Circle, { .position = pos_circle, .dimensions = { { 1.f, 1.f, 1.f } } });

			renderer.submit_batched_geometry(*command_executor);
			renderer.end_pass(*command_executor);
		}
		command_executor->submit_and_end();
	}

	void AppLayer::destruct() { command_executor.reset(); }

} // namespace Disarray::Client
