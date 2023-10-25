#include <Disarray.hpp>
#include <ImGuizmo.h>

#include <array>
#include <vector>

#include "core/ThreadPool.hpp"
#include "core/UsageBadge.hpp"

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

	void create_entities();

private:
	void handle_file_drop(const std::filesystem::path&);

	Extent extent {};

	Scope<Scene> scene;
	Scope<Scene> running_scene;
	Device& device;

	EditorCamera camera;
	GizmoType gizmo_type { GizmoType::Rotate };

	bool viewport_panel_mouse_over { false };
	bool viewport_panel_focused { false };
	std::array<glm::vec2, 2> viewport_bounds {};
	std::array<glm::vec2, 2> vp_bounds {};
};

} // namespace Disarray::Client
