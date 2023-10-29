#pragma once

#include <Disarray.hpp>
#include <entt/entt.hpp>

#include "core/KeyCode.hpp"
#include "core/Panel.hpp"
#include "scene/Component.hpp"
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

	template <ValidComponent ToTest, ValidComponent... CompareWith> auto is_deletable_component(ComponentGroup<CompareWith...>)
	{
		return (std::is_same_v<ToTest, CompareWith> || ...);
	}

	template <ValidComponent T, class Func>
	void draw_component(Entity& entity, std::string_view name, Func&& ui_function, Ref<Disarray::Texture> icon = nullptr)
	{
		if (!entity.has_component<T>()) {
			return;
		}

		ImGui::PushID((void*)typeid(T).hash_code());
		ImVec2 content_region_available = ImGui::GetContentRegionAvail();

		bool open = ImGui::TreeNodeEx(name.data(), ImGuiTreeNodeFlags_OpenOnDoubleClick);
		bool right_clicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);
		float line_height = ImGui::GetItemRectMax().y - ImGui::GetItemRectMin().y;

		bool reset_values = false;
		bool remove_component = false;

		ImGui::SameLine(content_region_available.x - line_height - 5.0f);
		UI::shift_cursor_y(line_height / 4.0F);
		if (ImGui::Button("+", ImVec2 { line_height, line_height }) || right_clicked) {
			ImGui::OpenPopup("ComponentSettings");
		}

		if (ImGui::BeginPopup("ComponentSettings")) {
			auto& component = entity.get_components<T>();

			if (ImGui::MenuItem("Reset")) {
				reset_values = true;
			}

			if (!is_deletable_component<T>(NonDeletableComponents {})) {
				if (ImGui::MenuItem("Remove component")) {
					remove_component = true;
				}
			}

			ImGui::EndPopup();
		}

		if (open) {
			T& component = entity.get_components<T>();
			std::forward<Func>(ui_function)(component);
			ImGui::TreePop();
		}

		if (remove_component) {
			if (entity.has_component<T>()) {
				entity.remove_component<T>();
			}
		}

		if (reset_values) {
			if (entity.has_component<T>()) {
				entity.remove_component<T>();
				entity.add_component<T>();
			}
		}

		if (!open) {
			UI::shift_cursor_y(-(ImGui::GetStyle().ItemSpacing.y + 1.0F));
		}

		ImGui::PopID();
	}

	template <ValidComponent T, class Func> void draw_component(Entity& entity, Func&& ui_function, Ref<Disarray::Texture> icon = nullptr)
	{
		draw_component<T, Func>(entity, Components::component_name<T>, std::forward<Func>(ui_function), icon);
	}

	template <ValidComponent T> void draw_add_component_entry()
	{
		if (auto entity = Entity { scene, *selected_entity }; entity.is_valid() && !entity.has_component<T>()) {
			if (ImGui::MenuItem(Components::component_name<T>.data())) {
				entity.add_component<T>();
				ImGui::CloseCurrentPopup();
			}
		}
	}

	template <ValidComponent... T> void draw_add_component_all(ComponentGroup<T...>) { (draw_add_component_entry<T>(), ...); }
};

} // namespace Disarray::Client
