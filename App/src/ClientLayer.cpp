#include "ClientLayer.hpp"

#include "panels/DirectoryContentPanel.hpp"
#include "panels/ExecutionStatisticsPanel.hpp"
#include "panels/ScenePanel.hpp"
#include "panels/StatisticsPanel.hpp"

#include <Disarray.hpp>
#include <array>
#include <fmt/format.h>
#include <glm/gtx/matrix_decompose.hpp>

namespace Disarray::Client {

ClientLayer::ClientLayer(Device& device, Window& win, Swapchain& swapchain)
	: device(device)
	, camera(60.f, static_cast<float>(swapchain.get_extent().width), static_cast<float>(swapchain.get_extent().height), 0.1f, 1000.f, nullptr)
{
}

ClientLayer::~ClientLayer() = default;

void ClientLayer::construct(App& app, Renderer& renderer, ThreadPool& pool)
{
	auto test_scene = Scene::deserialise(device, "Default scene", "Assets/Scene/Default_scene-2023-08-12-11-43-08.json");
	scene.reset(new Scene { device, "Default scene" });

	ensure(scene != nullptr, "Forgot to initialise scene");
	scene->construct(app, renderer, pool);

	auto stats_panel = app.add_panel<StatisticsPanel>(app.get_statistics());
	auto content_panel = app.add_panel<DirectoryContentPanel>("Assets");
	auto scene_panel = app.add_panel<ScenePanel>(*scene);
	auto execution_stats_panel = app.add_panel<ExecutionStatisticsPanel>(scene->get_command_executor());

	stats_panel->construct(app, renderer, pool);
	content_panel->construct(app, renderer, pool);
	scene_panel->construct(app, renderer, pool);
	execution_stats_panel->construct(app, renderer, pool);
};

void ClientLayer::interface()
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

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 6.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);
	ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4 { 0.0f, 0.0f, 0.0f, 0.0f });
	ImGui::Begin("Dockspace", nullptr, window_flags);
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);

	ImGui::PopStyleVar(2);

	float min_win_size_x = style.WindowMinSize.x;
	style.WindowMinSize.x = 370.0f;
	ImGui::DockSpace(ImGui::GetID("Dockspace"));
	style.WindowMinSize.x = min_win_size_x;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("Viewport");
	{
		viewport_panel_mouse_over = ImGui::IsWindowHovered();
		viewport_panel_focused = ImGui::IsWindowFocused();

		auto viewport_offset = ImGui::GetCursorPos(); // includes tab bar
		auto viewport_size = ImGui::GetContentRegionAvail();
		// camera.set_viewport_size<FloatExtent>({ viewport_size.x, viewport_size.y });

		auto& image = scene->get_image(0);
		UI::image(image, { viewport_size.x, viewport_size.y });

		if (auto& entity = scene->get_selected_entity(); entity->is_valid()) {
			scene->manipulate_entity_transform(*entity, camera, gizmo_type);
		}

		auto window_size = ImGui::GetWindowSize();
		ImVec2 min_bound = ImGui::GetWindowPos();

		min_bound.x -= viewport_offset.x;
		min_bound.y -= viewport_offset.y;

		ImVec2 max_bound = { min_bound.x + window_size.x, min_bound.y + window_size.y };
		viewport_bounds[0] = { min_bound.x, min_bound.y };
		viewport_bounds[1] = { max_bound.x, max_bound.y };

		vp_bounds = { glm::vec2 { viewport_bounds[1].x - viewport->Pos.x, viewport_bounds[1].y - viewport->Pos.y },
			{ viewport_bounds[0].x - viewport->Pos.x, viewport_bounds[0].y - viewport->Pos.y } };

		scene->set_viewport_bounds(vp_bounds[0], vp_bounds[1]);
	}
	ImGui::End();
	ImGui::PopStyleVar();

	auto& depth_image = scene->get_image(2);
	UI::scope("Depth"sv, [&depth_image]() {
		auto viewport_size = ImGui::GetContentRegionAvail();
		UI::image(depth_image, { viewport_size.x, viewport_size.y });
	});

	ImGui::End();
}

void ClientLayer::handle_swapchain_recreation(Swapchain& swapchain) { scene->recreate(swapchain.get_extent()); }

void ClientLayer::on_event(Event& event)
{
	EventDispatcher dispatcher { event };
	dispatcher.dispatch<KeyReleasedEvent>([this](auto& event) {
		switch (event.get_key_code()) {
		case T: {
			if (gizmo_type == GizmoType::Translate) {
				gizmo_type = GizmoType::Rotate;
			} else if (gizmo_type == GizmoType::Rotate) {
				gizmo_type = GizmoType::Scale;
			} else {
				gizmo_type = GizmoType::Translate;
			}
			return false;
		}
		default:
			return false;
		}
	});

	dispatcher.dispatch<MouseButtonReleasedEvent>([this](MouseButtonReleasedEvent& pressed) {
		if (ImGuizmo::IsUsing())
			return true;

		if (pressed.get_mouse_button() == MouseCode::Left) {
			const auto& image = scene->get_image(1);
			auto pos = Input::mouse_position();
			pos -= vp_bounds[1];

			pos.x /= (vp_bounds[0].x - vp_bounds[1].x);
			pos.y /= vp_bounds[0].y;

			if (pos.x < 0 || pos.x > 1 || pos.y < 0 || pos.y > 1) {
				scene->update_picked_entity(0);
				return true;
			}

			auto pixel_data = image.read_pixel(pos);
			std::visit(
				Tuple::overload { [&s = this->scene](std::uint32_t& handle) {
									 Log::info("Client Layer", "Entity data: {}", handle);
									 s->update_picked_entity(handle);
								 },
					[](glm::vec4& vec) { Log::info("Client Layer", "Pixel data: {},{},{},{}", vec.x, vec.y, vec.z, vec.w); }, [](std::monostate) {} },
				pixel_data);
		}
		return false;
	});
	scene->on_event(event);
}

void ClientLayer::update(float ts)
{
	scene->update(ts);
	camera.on_update(ts);
}

void ClientLayer::render(Disarray::Renderer& renderer)
{
	renderer.begin_frame(camera);
	scene->render(renderer);
	renderer.end_frame();
}

void ClientLayer::destruct() { scene->destruct(); }

} // namespace Disarray::Client
