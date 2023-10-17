#pragma once

#include <Disarray.hpp>
#include <entt/entt.hpp>

#include "core/Panel.hpp"
#include "scene/Components.hpp"
#include "ui/UI.hpp"

namespace Disarray::Client {

class ScenePanel : public Disarray::Panel {
public:
	ScenePanel(Disarray::Device& dev, Disarray::Window&, Disarray::Swapchain&, Disarray::Scene* s);

	void update(float) override;
	void interface() override;
	void for_all_components(Entity& entity);
	void on_event(Event&) override;

private:
	void draw_entity_node(Entity&, bool has_parent, std::uint32_t depth = 0);

	Device& device;
	Scene* scene;

	std::unique_ptr<entt::entity> selected_entity {};

	template <ValidComponent TComponent, class UIFunction>
	void draw_component(Entity& entity, std::string_view name, UIFunction&& uiFunction, Ref<Disarray::Texture> icon = nullptr)
	{
		if (!entity.has_component<TComponent>()) {
			return;
		}

		ImGui::PushID((void*)typeid(TComponent).hash_code());
		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		bool open = ImGui::TreeNodeEx(name.data(), ImGuiTreeNodeFlags_OpenOnDoubleClick);
		bool right_clicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);
		float lineHeight = ImGui::GetItemRectMax().y - ImGui::GetItemRectMin().y;

		bool reset_values = false;
		bool remove_component = false;

		ImGui::SameLine(contentRegionAvailable.x - lineHeight - 5.0f);
		UI::shift_cursor_y(lineHeight / 4.0f);
		if (ImGui::Button("+", ImVec2 { lineHeight, lineHeight }) || right_clicked) {
			ImGui::OpenPopup("ComponentSettings");
		}

		if (ImGui::BeginPopup("ComponentSettings")) {
			auto& component = entity.get_components<TComponent>();

			if (ImGui::MenuItem("Reset")) {
				reset_values = true;
			}

			if constexpr (!std::is_same_v<TComponent, Components::Transform>) {
				if (ImGui::MenuItem("Remove component")) {
					remove_component = true;
				}
			}

			ImGui::EndPopup();
		}

		if (open) {
			TComponent& component = entity.get_components<TComponent>();
			std::forward<UIFunction>(uiFunction)(component);
			ImGui::TreePop();
		}

		if (remove_component) {
			if (entity.has_component<TComponent>()) {
				entity.remove_component<TComponent>();
			}
		}

		if (reset_values) {
			if (entity.has_component<TComponent>()) {
				entity.remove_component<TComponent>();
				entity.add_component<TComponent>();
			}
		}

		if (!open)
			UI::shift_cursor_y(-(ImGui::GetStyle().ItemSpacing.y + 1.0f));

		ImGui::PopID();
	}
};

} // namespace Disarray::Client
