#include "ClientLayer.hpp"

#include "glm/ext/matrix_transform.hpp"
#include "panels/ExecutionStatisticsPanel.hpp"

#include <Disarray.hpp>
#include <array>

namespace Disarray::Client {

	AppLayer::AppLayer(Device& dev, Window& win, Swapchain& swap)
		: window(win)
		, swapchain(swap)
		, scene(dev, win, swap, "Default scene")
	{
		(void)window.native();
	};

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
		scene.construct(app, renderer, pool);

		app.add_panel<StatisticsPanel>(app.get_statistics());
		app.add_panel<B>();
		app.add_panel<C>();
		app.add_panel<ExecutionStatisticsPanel>(scene.get_command_executor());
	};

	void AppLayer::interface()
	{
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
			auto& image = scene.get_framebuffer().get_image();
			UI::image(image, { viewport_size.x, viewport_size.y });

			auto window_size = ImGui::GetWindowSize();
			ImVec2 min_bound = ImGui::GetWindowPos();

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

	void AppLayer::handle_swapchain_recreation(Renderer& renderer) { scene.recreate(swapchain.get_extent()); }

	void AppLayer::on_event(Event& event) { scene.on_event(event); }

	void AppLayer::update(float ts, Renderer& renderer)
	{
		scene.update(ts);
		camera.on_update(ts);

		renderer.begin_frame(camera);
		scene.render(renderer);
		renderer.end_frame();
	}

	void AppLayer::destruct() { scene.destruct(); }

} // namespace Disarray::Client
