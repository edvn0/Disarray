#pragma once

#include <Disarray.hpp>
#include <entt/entt.hpp>

#include "core/Panel.hpp"
#include "scene/Components.hpp"
#include "ui/UI.hpp"

namespace Disarray::Client {

class ScenePanel : public Disarray::Panel {
public:
	ScenePanel(Disarray::Device& dev, Disarray::Window&, Disarray::Swapchain&, Disarray::Scene& s);

	void update(float) override;
	void interface() override;
	void for_all_components(Entity& entity);
	void on_event(Event&) override;

private:
	void draw_entity_node(Entity&, bool has_parent, std::uint32_t depth = 0);

	Device& device;
	Scene& scene;

	std::unique_ptr<entt::entity> selected_entity {};
};

} // namespace Disarray::Client
