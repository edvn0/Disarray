#include "ClientLayer.hpp"

#include <glm/gtx/matrix_decompose.hpp>

#include <Disarray.hpp>
#include <fmt/format.h>

#include <array>
#include <exception>
#include <filesystem>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>

#include "core/Random.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/RendererProperties.hpp"
#include "imgui_internal.h"
#include "panels/DirectoryContentPanel.hpp"
#include "panels/ExecutionStatisticsPanel.hpp"
#include "panels/LogPanel.hpp"
#include "panels/PipelineEditorPanel.hpp"
#include "panels/ScenePanel.hpp"
#include "panels/StatisticsPanel.hpp"
#include "scene/Components.hpp"
#include "scene/Scene.hpp"
#include "util/Timer.hpp"

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

	scene = make_scope<Scene>(device, "Default scene");
	scene->construct(app);
	create_entities();

	extent = app.get_swapchain().get_extent();
	auto copied = Scene::copy(*scene);
	scene = std::move(copied);

	auto stats_panel = app.add_panel<StatisticsPanel>(app.get_statistics());
	auto content_panel = app.add_panel<DirectoryContentPanel>("Assets");
	auto scene_panel = app.add_panel<ScenePanel>(*scene);
	auto execution_stats_panel = app.add_panel<ExecutionStatisticsPanel>(scene_renderer.get_command_executor());
	auto pipeline_editor_panel = app.add_panel<PipelineEditorPanel>(scene_renderer.get_pipeline_cache());
	// auto log_panel = app.add_panel<LogPanel>("Assets/Logs/disarray_errors.log");

	stats_panel->construct(app);
	content_panel->construct(app);
	scene_panel->construct(app);
	execution_stats_panel->construct(app);
	pipeline_editor_panel->construct(app);
	// log_panel->construct(app);
}

void ClientLayer::setup_filewatcher_and_threadpool(Threading::ThreadPool& pool)
{
	static constexpr auto tick_time = std::chrono::milliseconds(3000);
	file_watcher = make_scope<FileWatcher>(pool, "Assets/Shaders", Collections::StringSet { ".vert", ".frag", ".glsl" }, tick_time);
	auto& pipeline_cache = scene_renderer.get_pipeline_cache();
	file_watcher->on_modified([&ext = extent, &pipeline_cache](const FileInformation& entry) { pipeline_cache.force_recreation(ext); });
}

void ClientLayer::create_entities()
{
	auto& graphics_resource = scene_renderer.get_graphics_resource();

	const auto cube_mesh = Mesh::construct(device,
		MeshProperties {
			.path = FS::model("cube.obj"),
		});

	const VertexLayout layout {
		{ ElementType::Float3, "position" },
		{ ElementType::Float2, "uv" },
		{ ElementType::Float4, "colour" },
		{ ElementType::Float3, "normals" },
		{ ElementType::Float3, "tangents" },
		{ ElementType::Float3, "bitangents" },
	};
	auto& resources = graphics_resource;
	const auto& desc_layout = resources.get_descriptor_set_layouts();

	auto texture_cube = Texture::construct(device,
		{
			.path = FS::texture("cubemap_yokohama_rgba.ktx"),
			.dimension = TextureDimension::Three,
			.debug_name = "Skybox",
		});
	resources.expose_to_shaders(texture_cube->get_image(), DescriptorSet(2), DescriptorBinding(2));
	auto skybox_material = Material::construct(device,
		{
			.vertex_shader = scene_renderer.get_pipeline_cache().get_shader("skybox.vert"),
			.fragment_shader = scene_renderer.get_pipeline_cache().get_shader("skybox.frag"),
			.textures = { texture_cube },
		});

	auto environment = scene->create("Environment");
	environment.add_component<Components::Material>(skybox_material);
	environment.add_component<Components::Skybox>(texture_cube);
	environment.add_component<Components::Mesh>(cube_mesh);

	auto floor = scene->create("Floor");
	floor.get_transform().scale = { 70, 1, 70 };
	floor.get_transform().position = { 0, 7, 0 };
	floor.add_component<Components::BoxCollider>();

	floor.get_components<Components::ID>().can_interact_with = false;
	floor.add_component<Components::Texture>(nullptr, glm::vec4 { .1, .1, .9, 1.0 });
	floor.add_component<Components::Mesh>(cube_mesh);
	auto unit_vectors = scene->create("UnitVectors");
	const glm::vec3 base_pos { 0, 0, 0 };
	{
		auto axis = scene->create("XAxis");
		axis.get_components<Components::ID>().can_interact_with = false;
		auto& transform = axis.get_components<Components::Transform>();
		transform.position = base_pos;
		axis.add_component<Components::LineGeometry>(base_pos + glm::vec3 { 10.0, 0, 0 });
		axis.add_component<Components::Texture>(glm::vec4 { 1, 0, 0, 1 });
		unit_vectors.add_child(axis);
	}
	{
		auto axis = scene->create("YAxis");
		axis.get_components<Components::ID>().can_interact_with = false;
		auto& transform = axis.get_components<Components::Transform>();
		transform.position = base_pos;
		axis.add_component<Components::LineGeometry>(base_pos + glm::vec3 { 0, -10.0, 0 });
		axis.add_component<Components::Texture>(glm::vec4 { 0, 1, 0, 1 });
		unit_vectors.add_child(axis);
	}
	{
		auto axis = scene->create("ZAxis");
		axis.get_components<Components::ID>().can_interact_with = false;
		auto& transform = axis.get_components<Components::Transform>();
		transform.position = base_pos;
		axis.add_component<Components::LineGeometry>(base_pos + glm::vec3 { 0, 0, -10.0 });
		axis.add_component<Components::Texture>(glm::vec4 { 0, 0, 1, 1 });
		unit_vectors.add_child(axis);
	}

	auto unit_squares = scene->create("UnitSquares");

	static constexpr auto offset = glm::vec3(3, 3, 1);
	static constexpr auto squares = 10;
	static constexpr auto half_extent = (squares / 2) - 1;
	for (auto i = -squares / 2; i < half_extent; i++) {
		for (auto j = -squares / 2; j < half_extent; j++) {
			auto axis = scene->create("Square-x{}y{}", i, j);
			auto& transform = axis.add_component<Components::Transform>();
			transform.position = glm::vec3 { i, -7, j };
			transform.scale /= offset;
			transform.rotation = glm::angleAxis(glm::radians(90.0F), glm::vec3 { 1, 0, 0 });
			axis.add_component<Components::QuadGeometry>();
			axis.add_component<Components::Texture>(Random::strong_colour());
			unit_squares.add_child(axis);
		}
	}

	auto viking_rotation = Maths::rotate_by(glm::radians(glm::vec3 { 0, 0, 0 }));
	auto v_mesh = scene->create("Viking");
	const auto viking = Mesh::construct(device,
		{
			.path = FS::model("viking.obj"),
			.initial_rotation = viking_rotation,
		});
	v_mesh.get_components<Components::Transform>().position.y = -2;
	v_mesh.add_component<Components::Mesh>(viking);
	v_mesh.add_component<Components::Texture>(scene_renderer.get_texture_cache().get("viking_room"));

	auto viking_room_texture = Texture::construct(device,
		{
			.extent = extent,
			.format = ImageFormat::SBGR,
			.path = FS::texture("viking_room.png"),
			.debug_name = "viking",
		});
	v_mesh.add_component<Components::Texture>(viking_room_texture);
	static constexpr auto val = 10.0F;
	v_mesh.add_script<Scripts::LinearMovementScript>(-val, val);

	auto colours = generate_colours<50>();
	const auto sphere = Mesh::construct(device,
		{
			.path = FS::model("sphere.fbx"),
		});

	auto sun = scene->create("Sun");
	auto& directional_sun = sun.add_component<Components::DirectionalLight>(glm::vec4 { 255, 255, 255, 8 },
		Components::DirectionalLight::ProjectionParameters {
			.factor = 145.F,
			.near = -190.F,
			.far = 90.F,
			.fov = 60.F,
		});
	directional_sun.diffuse = Maths::scale_colour({ 49, 22, 22, 255 });
	directional_sun.specular = Maths::scale_colour({ 181, 255, 0, 255 });
	sun.add_component<Components::Mesh>(sphere);
	sun.add_component<Components::Transform>().position = { -15, -15, -16 };
	sun.add_component<Components::Controller>();

	auto pl_system = scene->create("PointLightSystem");

	constexpr auto create_point_light = [](Scope<Scene>& point_light_scene, auto index, auto& pl_sys, auto& cols, auto& sphere_mesh) {
		auto point_light = point_light_scene->create("PointLight-{}", index);
		point_light.template get_components<Components::ID>().can_interact_with = false;
		auto& light_component = point_light.template add_component<Components::PointLight>();
		light_component.ambient = cols.at(index).at(0);
		light_component.diffuse = cols.at(index).at(1);
		light_component.specular = cols.at(index).at(2);

		constexpr auto float_radius = static_cast<float>(point_light_radius);
		const auto point_in_sphere = Random::on_sphere(3.0F * float_radius);
		auto& transform = point_light.template get_components<Components::Transform>();
		transform.position = point_in_sphere;
		// transform.scale *= 2;

		point_light.template add_component<Components::Mesh>(sphere_mesh);
		point_light.template add_component<Components::Texture>(cols.at(index).at(0));
		pl_sys.add_child(point_light);
	};

	for (std::uint32_t i = 0; i < colours.size(); i++) {
		create_point_light(scene, i, pl_system, colours, sphere);
	}
	// sponza
	auto sponza = scene->create("Sponza");
	// Text
	auto text_hello = scene->create("Hello Text");
	auto& component = text_hello.add_component<Components::Text>();
	component.set_text("{}", "Hello world!");

	auto scene_camera = scene->create("Scene Camera");
	scene_camera.add_component<Components::Camera>();
	scene_camera.get_components<Components::Transform>().position = { 15, -16, 16 };
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

		if (const auto& entity = scene->get_selected_entity(); entity->is_valid()) {
			scene->manipulate_entity_transform(*entity, camera, gizmo_type);
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

	scene_renderer.interface();
	scene->interface();
}

void ClientLayer::handle_swapchain_recreation(Swapchain& swapchain)
{
	extent = swapchain.get_extent();
	scene->recreate(swapchain.get_extent());
	scene_renderer.recreate(true, swapchain.get_extent());
	auto& graphics_resource = scene_renderer.get_graphics_resource();

	auto texture_cube = scene->get_by_components<Components::Skybox>()->get_components<Components::Skybox>().texture;
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
		if (ImGuizmo::IsUsing()) {
			return true;
		}

		const auto vp_is_focused = viewport_panel_focused && viewport_panel_mouse_over;
		if (pressed.get_mouse_button() == MouseCode::Left && vp_is_focused) {
			const auto& image = scene_renderer.get_framebuffer<SceneFramebuffer::Identity>()->get_image(0);
			auto pos = Input::mouse_position();
			pos -= vp_bounds[1];

			pos.x /= (vp_bounds[0].x - vp_bounds[1].x);
			pos.y /= vp_bounds[0].y;

			if (pos.x < 0 || pos.x > 1 || pos.y < 0 || pos.y > 1) {
				scene->update_picked_entity(0);
				return true;
			}

			auto pixel_data = image.read_pixel(pos);
			std::visit(Tuple::overload { [&s = this->scene](const std::uint32_t& handle) { s->update_picked_entity(handle); },
						   [](const glm::vec4& vec) {}, [](std::monostate) {} },
				pixel_data);
			return true;
		}

		scene->update_picked_entity(0);
		return false;
	});
	camera.on_event(event);
	scene->on_event(event);
}

void ClientLayer::update(float time_step)
{
	camera.on_update(time_step);
	scene->update(time_step);
}

void ClientLayer::render()
{
	scene_renderer.begin_execution();
	scene->begin_frame(camera, scene_renderer);
	scene->render(scene_renderer);
	scene->end_frame(scene_renderer);
	scene_renderer.submit_executed_commands();
}

void ClientLayer::destruct()
{
	scene_renderer.destruct();
	file_watcher.reset();
	scene->destruct();
}

template <typename Child> class FileHandlerBase {
public:
	explicit FileHandlerBase(const Device& dev, Scene* scene, std::filesystem::path path)
		: device(dev)
		, base_scene(scene)
		, file_path(std::move(path))
	{
		if (valid_file()) {
			handle();
		}
	}

protected:
	auto child() -> auto& { return static_cast<Child&>(*this); }
	auto get_scene() -> auto* { return base_scene; }
	[[nodiscard]] auto get_device() const -> const auto& { return device; }
	[[nodiscard]] auto get_path() const -> const auto& { return file_path; }

private:
	void handle() { return child().handle_impl(); }
	auto valid_file() -> bool { return child().valid_file_impl(); }
	const Device& device;
	Scene* base_scene;
	std::filesystem::path file_path {};
	Collections::StringViewSet extensions {};
	bool valid { false };
};

class PNGHandler : public FileHandlerBase<PNGHandler> {
public:
	using FileHandlerBase<PNGHandler>::FileHandlerBase;

private:
	void handle_impl()
	{
		auto* scene = get_scene();
		const auto& path = get_path();
		auto entity = scene->create(path.filename().string());
		auto texture = Texture::construct(get_device(),
			{
				.path = path,
				.debug_name = path.filename().string(),
			});

		entity.add_component<Components::Texture>(texture);
	}

	auto valid_file_impl() -> bool
	{
		using namespace std::string_view_literals;
		const auto& path = get_path();
		static std::unordered_set valid_extensions = { ".png"sv, ".jpg"sv, ".png"sv, ".jpeg"sv, ".bmp"sv };
		return valid_extensions.contains(path.extension().string());
	}

	friend class FileHandlerBase<PNGHandler>;
};

class MeshHandler : public FileHandlerBase<MeshHandler> {
public:
	using FileHandlerBase<MeshHandler>::FileHandlerBase;

private:
	void handle_impl()
	{
		auto* scene = get_scene();
		const auto& path = get_path();
		auto entity = scene->create(path.filename().string());
		MeshProperties properties {
			.path = path,
		};
		auto mesh = Mesh::construct(get_device(), properties);

		entity.add_component<Components::Mesh>(mesh);
	}

	auto valid_file_impl() -> bool
	{
		using namespace std::string_view_literals;
		const auto& path = get_path();
		static std::unordered_set valid_extensions = {
			".mesh"sv,
			".obj"sv,
			".fbx"sv,
		};
		return valid_extensions.contains(path.extension().string());
	}

	friend class FileHandlerBase<MeshHandler>;
};

class SceneHandler : public FileHandlerBase<SceneHandler> {
public:
	using FileHandlerBase<SceneHandler>::FileHandlerBase;

private:
	void handle_impl()
	{
		auto& current_scene = *get_scene();
		current_scene.clear();
		Scene::deserialise_into(current_scene, get_device(), get_path());
	}

	auto valid_file_impl() -> bool
	{
		using namespace std::string_view_literals;
		const auto& path = get_path();
		static std::unordered_set valid_extensions = { ".scene"sv, ".json"sv };
		return valid_extensions.contains(path.extension().string());
	}

	friend class FileHandlerBase<SceneHandler>;
};

namespace {
	template <class... T> struct HandlerGroup { };

} // namespace

void ClientLayer::handle_file_drop(const std::filesystem::path& path)
{
	if (!std::filesystem::exists(path)) {
		return;
	}

	static constexpr auto evaluate_all = []<class... T>(HandlerGroup<T...>, const auto& dev, auto* scene_ptr, const auto& path) {
		(T { dev, scene_ptr, path }, ...);
	};

	using FileHandlers = HandlerGroup<PNGHandler, SceneHandler, MeshHandler>;
	evaluate_all(FileHandlers {}, device, scene.get(), path);
}

} // namespace Disarray::Client
