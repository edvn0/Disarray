#include <Disarray.hpp>
#include <ImGuizmo.h>

#include <array>

#include "core/ThreadPool.hpp"
#include "panels/ScenePanel.hpp"
#include "scene/Scene.hpp"

namespace Disarray::Client {

class ClientLayer : public Layer {
public:
	ClientLayer(Device&, Window&, Swapchain&);
	~ClientLayer() override;

	void interface() override;

	void construct(App& /*unused*/) override;
	void handle_swapchain_recreation(Swapchain& /*unused*/) override;
	void on_event(Event& /*unused*/) override;
	void update(float /*time_step*/) override;
	void render() override;
	void destruct() override;

private:
	void handle_file_drop(const std::filesystem::path&);
	void setup_filewatcher_and_threadpool(Threading::ThreadPool& pool);

	auto toolbar() -> void;
	auto on_scene_play() -> void;
	auto on_scene_stop() -> void;
	auto on_scene_pause() -> void;
	auto on_scene_unpause() -> void;
	auto on_scene_simulate() -> void;

	Device& device;
	Extent extent {};

	SceneRenderer scene_renderer;

	Ref<Scene> scene;
	Ref<Scene> running_scene;
	SceneState scene_state { SceneState::Edit };
	Scope<FileWatcher> file_watcher;

	std::shared_ptr<ScenePanel> scene_panel;

	Ref<Disarray::Texture> icon_play { nullptr };
	Ref<Disarray::Texture> icon_stop { nullptr };
	Ref<Disarray::Texture> icon_pause { nullptr };
	Ref<Disarray::Texture> icon_step { nullptr };
	Ref<Disarray::Texture> icon_simulate { nullptr };

	EditorCamera camera;
	GizmoType gizmo_type { GizmoType::Rotate };

	bool viewport_panel_mouse_over { false };
	bool viewport_panel_focused { false };
	std::array<glm::vec2, 2> viewport_bounds {};
	std::array<glm::vec2, 2> vp_bounds {};
};

} // namespace Disarray::Client
