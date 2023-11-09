#include "ClientLayer.hpp"

#include <glm/gtx/matrix_decompose.hpp>

#include <Disarray.hpp>
#include <fmt/format.h>
#include <imgui_internal.h>

#include <array>
#include <filesystem>
#include <initializer_list>
#include <memory>
#include <utility>

#include "core/Random.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/RendererProperties.hpp"
#include "panels/DirectoryContentPanel.hpp"
#include "panels/ExecutionStatisticsPanel.hpp"
#include "panels/PipelineEditorPanel.hpp"
#include "panels/StatisticsPanel.hpp"
#include "scene/Components.hpp"
#include "scene/Scene.hpp"
#include "ui/UI.hpp"

namespace {
template <std::size_t Count>
	requires(Count <= Disarray::max_point_lights)
auto generate_colours() -> std::array<std::array<glm::vec4, 4>, Count>
{
	std::array<std::array<glm::vec4, 4>, Count> colours {};
	for (std::size_t i = 0; i < Count; i++) {
		colours.at(i).at(0) = Disarray::Random::colour();
		colours.at(i).at(1) = Disarray::Random::colour();
		colours.at(i).at(2) = Disarray::Random::colour();
		colours.at(i).at(3) = Disarray::Random::colour();
	}
	return colours;
}
} // namespace
namespace Disarray::Client {

ClientLayer::ClientLayer(Device& device, Window& win, Swapchain& swapchain)
	: device(device)
	, scene_renderer(device)
	, camera(60.F, static_cast<float>(swapchain.get_extent().width), static_cast<float>(swapchain.get_extent().height), 0.1F, 1000.F, nullptr)
{
}

ClientLayer::~ClientLayer() = default;

void ClientLayer::construct(App& app)
{
	scene_renderer.construct(app);
	setup_filewatcher_and_threadpool(app.get_thread_pool());

	scene = make_ref<Scene>(device, "Default scene");
	scene->construct(app);
	icon_play = scene_renderer.get_texture_cache().get("Play");
	icon_stop = scene_renderer.get_texture_cache().get("Stop");
	icon_pause = scene_renderer.get_texture_cache().get("Pause");
	icon_step = scene_renderer.get_texture_cache().get("Step");
	icon_simulate = scene_renderer.get_texture_cache().get("Simulate");
	// create_entities();

	extent = app.get_swapchain().get_extent();
	running_scene = scene;
	running_scene->sort();

	auto stats_panel = app.add_panel<StatisticsPanel>(app.get_statistics());
	auto content_panel = app.add_panel<DirectoryContentPanel>("Assets");
	auto execution_stats_panel = app.add_panel<ExecutionStatisticsPanel>(scene_renderer.get_command_executor());
	auto pipeline_editor_panel = app.add_panel<PipelineEditorPanel>(scene_renderer.get_pipeline_cache());
	scene_panel = std::dynamic_pointer_cast<ScenePanel>(app.add_panel<ScenePanel>(running_scene.get()));
	// auto log_panel = app.add_panel<LogPanel>("Assets/Logs/disarray_errors.log");

	stats_panel->construct(app);
	content_panel->construct(app);
	execution_stats_panel->construct(app);
	pipeline_editor_panel->construct(app);
	scene_panel->construct(app);
	// log_panel->construct(app);
}

void ClientLayer::setup_filewatcher_and_threadpool(Threading::ThreadPool& pool)
{
	static constexpr auto tick_time = std::chrono::milliseconds(3000);
	file_watcher = make_scope<FileWatcher>(pool, "Assets/Shaders", Collections::StringSet { ".vert", ".frag", ".glsl" }, tick_time);
	auto& pipeline_cache = scene_renderer.get_pipeline_cache();
	file_watcher->on_modified([&ext = extent, &pipeline_cache](const FileInformation& entry) { pipeline_cache.force_recreation(ext); });
}

auto ClientLayer::toolbar() -> void
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	auto& colors = ImGui::GetStyle().Colors;
	const auto& button_hovered = colors[ImGuiCol_ButtonHovered];
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(button_hovered.x, button_hovered.y, button_hovered.z, 0.5f));
	const auto& button_active = colors[ImGuiCol_ButtonActive];
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(button_active.x, button_active.y, button_active.z, 0.5f));

	ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	bool toolbar_enabled = running_scene != nullptr;

	float size = ImGui::GetWindowHeight() - 4.0f;
	ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));

	bool has_play_button = scene_state == SceneState::Edit || scene_state == SceneState::Play;
	bool has_simulate_button = scene_state == SceneState::Edit || scene_state == SceneState::Simulate;
	bool has_pause_button = scene_state != SceneState::Edit;

	if (has_play_button) {
		const auto& icon = (scene_state == SceneState::Edit || scene_state == SceneState::Simulate) ? icon_play : icon_stop;
		if (UI::image_button(*icon, glm::vec2(size, size)) && toolbar_enabled) {
			if (scene_state == SceneState::Edit || scene_state == SceneState::Simulate) {
				on_scene_play();
			} else {
				on_scene_stop();
			}
		}
	}

	if (has_simulate_button) {
		if (has_play_button) {
			ImGui::SameLine();
		}

		const auto& icon = (scene_state == SceneState::Edit || scene_state == SceneState::Play) ? icon_simulate : icon_stop;
		if (UI::image_button(*icon, glm::vec2(size, size)) && toolbar_enabled) {
			if (scene_state == SceneState::Edit || scene_state == SceneState::Play) {
				on_scene_simulate();
			} else {
				on_scene_stop();
			}
		}
	}
	if (has_pause_button) {
		bool is_paused = running_scene->is_paused();
		ImGui::SameLine();
		const auto& pause_icon = icon_pause;

		if (!is_paused) {
			if (UI::image_button(*pause_icon, glm::vec2(size, size)) && toolbar_enabled) {
				on_scene_pause();
			}
		} else {
			if (UI::image_button(*icon_play, glm::vec2(size, size)) && toolbar_enabled) {
				on_scene_unpause();
			}
		}

		// Step button
		if (is_paused) {
			ImGui::SameLine();
			const auto& icon = icon_step;
			if (UI::image_button(*icon, glm::vec2(size, size)) && toolbar_enabled) {
				running_scene->step();
			}
		}
	}
	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(3);
	ImGui::End();
}

void ClientLayer::on_scene_play()
{
	if (scene_state == SceneState::Simulate) {
		on_scene_stop();
	}

	scene_state = SceneState::Play;

	running_scene = Scene::copy(*scene);
	running_scene->on_runtime_start();

	scene_panel->set_scene(running_scene.get());
}

void ClientLayer::on_scene_simulate()
{
	if (scene_state == SceneState::Play) {
		on_scene_stop();
	}

	scene_state = SceneState::Simulate;

	running_scene = Scene::copy(*scene);
	running_scene->on_simulation_start();

	scene_panel->set_scene(running_scene.get());
}

void ClientLayer::on_scene_stop()
{
	ensure(scene_state == SceneState::Play || scene_state == SceneState::Simulate);

	if (scene_state == SceneState::Play) {
		running_scene->on_runtime_stop();
	} else if (scene_state == SceneState::Simulate) {
		running_scene->on_simulation_stop();
	}

	scene_state = SceneState::Edit;

	running_scene = scene;

	scene_panel->set_scene(running_scene.get());
}

void ClientLayer::on_scene_pause()
{
	if (scene_state == SceneState::Edit) {
		return;
	}

	running_scene->set_paused(true);
}

void ClientLayer::on_scene_unpause()
{
	if (scene_state == SceneState::Edit) {
		return;
	}

	running_scene->set_paused(false);
}

void ClientLayer::interface()
{
	ImGuiIO& imgui_io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();

	imgui_io.ConfigWindowsResizeFromEdges = ((imgui_io.BackendFlags & ImGuiBackendFlags_HasMouseCursors) != 0);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0F);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0F);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0F, 6.0F));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0F);
	ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4 { 0.0F, 0.0F, 0.0F, 0.0F });
	ImGui::Begin("Dockspace", nullptr, window_flags);
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);

	ImGui::PopStyleVar(2);

	const float min_win_size_x = style.WindowMinSize.x;
	style.WindowMinSize.x = 370.0F;
	ImGui::DockSpace(ImGui::GetID("Dockspace"));
	style.WindowMinSize.x = min_win_size_x;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("Viewport");
	{
		viewport_panel_mouse_over = ImGui::IsWindowHovered();
		viewport_panel_focused = ImGui::IsWindowFocused();

		auto viewport_offset = ImGui::GetCursorPos();
		auto viewport_size = ImGui::GetContentRegionAvail();
		camera.set_viewport_size(FloatExtent { viewport_size.x, viewport_size.y });

		const auto& image = scene_renderer.get_final_image();
		UI::image(image, { viewport_size.x, viewport_size.y });

		if (const auto& entity = running_scene->get_selected_entity(); entity->is_valid()) {
			running_scene->manipulate_entity_transform(*entity, camera, gizmo_type);
		}

		auto window_size = ImGui::GetWindowSize();
		ImVec2 min_bound = ImGui::GetWindowPos();

		const auto drag_dropped = UI::accept_drag_drop("Disarray::DragDropItem");
		if (drag_dropped) {
			handle_file_drop(*drag_dropped);
		}

		min_bound.x -= viewport_offset.x;
		min_bound.y -= viewport_offset.y;

		const ImVec2 max_bound = { min_bound.x + window_size.x, min_bound.y + window_size.y };
		viewport_bounds[0] = { min_bound.x, min_bound.y };
		viewport_bounds[1] = { max_bound.x, max_bound.y };

		vp_bounds = { glm::vec2 { viewport_bounds[1].x - viewport->Pos.x, viewport_bounds[1].y - viewport->Pos.y },
			{ viewport_bounds[0].x - viewport->Pos.x, viewport_bounds[0].y - viewport->Pos.y } };
	}
	ImGui::End();
	ImGui::PopStyleVar();

	const auto& depth_image = scene_renderer.get_framebuffer<SceneFramebuffer::Shadow>()->get_depth_image();
	UI::scope("Depth"sv, [&depth_image]() {
		auto viewport_size = ImGui::GetContentRegionAvail();
		UI::image(depth_image, { viewport_size.x, viewport_size.y });
	});

	ImGui::End();

	toolbar();

	scene_renderer.interface();
	running_scene->interface();
}

void ClientLayer::handle_swapchain_recreation(Swapchain& swapchain)
{
	extent = swapchain.get_extent();
	running_scene->recreate(swapchain.get_extent());
	scene_renderer.recreate(true, swapchain.get_extent());
	auto& graphics_resource = scene_renderer.get_graphics_resource();

	auto texture_cube = running_scene->get_by_components<Components::Skybox>()->get_components<Components::Skybox>().texture;
	graphics_resource.expose_to_shaders(texture_cube->get_image(), DescriptorSet(2), DescriptorBinding(2));
}

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
			return;
		}
		default:
			return;
		}
	});
	dispatcher.dispatch<MouseButtonReleasedEvent>([this](MouseButtonReleasedEvent& pressed) {
		running_scene->update_picked_entity(0);

		if (ImGuizmo::IsUsing()) {
			running_scene->update_picked_entity(0);
			return false;
		}

		const auto vp_is_focused = viewport_panel_focused && viewport_panel_mouse_over;
		if (pressed.get_mouse_button() == MouseCode::Left && vp_is_focused) {
			const auto& image = scene_renderer.get_framebuffer<SceneFramebuffer::Identity>()->get_image(0);
			auto pos = Input::mouse_position();
			pos -= vp_bounds[1];

			pos.x /= (vp_bounds[0].x - vp_bounds[1].x);
			pos.y /= vp_bounds[0].y;

			if (pos.x < 0 || pos.x > 1 || pos.y < 0 || pos.y > 1) {
				running_scene->update_picked_entity(0);
				return true;
			}

			auto pixel_data = image.read_pixel(pos);
			std::visit(Tuple::overload {
						   [&current_scene = running_scene](const std::uint32_t& handle) { current_scene->update_picked_entity(handle); },
						   [](const glm::vec4& vec) {},
						   [](std::monostate) {},
					   },
				pixel_data);
			return true;
		}

		return true;
	});
	camera.on_event(event);
	running_scene->on_event(event);
}

void ClientLayer::update(float time_step)
{
	switch (scene_state) {
	case SceneState::Edit: {
		camera.on_update(time_step);
		running_scene->on_update_editor(time_step);
		break;
	}
	case SceneState::Simulate: {
		camera.on_update(time_step);
		running_scene->on_update_simulation(time_step);
		break;
	}
	case SceneState::Play: {
		running_scene->on_update_runtime(time_step);
		break;
	}
	}
}

void ClientLayer::render()
{
	scene_renderer.begin_execution();
	if (scene_state == SceneState::Play) {
		auto primary_camera = scene->get_primary_camera();

		if (primary_camera.has_value()) {
			auto&& [view, projection, view_projection] = *primary_camera;
			running_scene->begin_frame(view, projection, view_projection, scene_renderer);
		} else {
			running_scene->begin_frame(camera, scene_renderer);
		}
	} else {
		running_scene->begin_frame(camera, scene_renderer);
	}
	running_scene->render(scene_renderer);
	running_scene->end_frame(scene_renderer);
	scene_renderer.submit_executed_commands();
}

void ClientLayer::destruct()
{
	scene_renderer.destruct();
	file_watcher.reset();
	scene->destruct();
}

#include "handlers/file_handlers.inl"

void ClientLayer::handle_file_drop(const std::filesystem::path& path)
{
	if (!std::filesystem::exists(path)) {
		return;
	}

	static constexpr auto evaluate_all = []<class... T>(HandlerGroup<T...>, const auto& dev, auto* scene_ptr, const auto& path) {
		(T { dev, scene_ptr, path }, ...);
	};

	evaluate_all(FileHandlers {}, device, scene.get(), path);
}

} // namespace Disarray::Client
