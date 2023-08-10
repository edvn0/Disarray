#include "panels/ScenePanel.hpp"

#include "graphics/Pipeline.hpp"

namespace Disarray::Client {

	template <ValidComponent T> void draw_component(Entity& entity, const std::string& name, auto&& ui_function)
	{
		if (!entity.has_component<T>())
			return;

		const ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth
			| ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
		auto& component = entity.get_components<T>();
		ImVec2 content_region_available = ImGui::GetContentRegionAvail();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2 { 4, 4 });
		float line_height = 10 + 2 + 1 * 2.0f;
		ImGui::Separator();

		const auto leaf_id = (const void*)typeid(T).hash_code();
		bool open = ImGui::TreeNodeEx(leaf_id, tree_node_flags, "%s", name.c_str());
		ImGui::PopStyleVar();
		ImGui::SameLine(content_region_available.x - line_height * 0.5f);
		if (ImGui::Button("+", ImVec2 { line_height, line_height })) {
			ImGui::OpenPopup("Component Settings");
		}

		bool remove_component = false;
		if (ImGui::BeginPopup("Component Settings")) {
			if (ImGui::MenuItem("Remove component"))
				remove_component = true;

			ImGui::EndPopup();
		}

		if (open) {
			ui_function(component);
			ImGui::TreePop();
		}

		if (remove_component)
			entity.remove_component<T>();
	}

	void ScenePanel::interface()
	{
		auto& registry = scene.get_registry();
		static bool debug_log_window_open { true };
		static bool window_open { true };
		ImGui::ShowDebugLogWindow(&debug_log_window_open);
		ImGui::ShowDemoWindow(&window_open);

		UI::begin("Scene");
		std::unordered_set<entt::entity> selectable_entities;
		if (ImGui::BeginTable("Entities", 2)) {
			const auto inheritance_view = registry.view<const ID, const Tag, const Inheritance>();
			for (const auto& [entity, id, tag, inheritance] : inheritance_view.each()) {
				selectable_entities.insert(entity);
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("%s", tag.name.c_str());
				ImGui::TableNextColumn();
				const auto& children = inheritance.children;
				auto text = fmt::format("{} - Children: [{}]", id.identifier, fmt::join(children, ", "));
				ImGui::Text("%s", text.c_str());
			}

			const auto view = registry.view<const ID, const Tag>(entt::exclude<Inheritance>);
			for (const auto& [entity, id, tag] : view.each()) {
				selectable_entities.insert(entity);
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("%s", tag.name.c_str());
				ImGui::TableNextColumn();
				ImGui::Text("%lu", id.identifier);
			}

			ImGui::EndTable();
		}

		UI::begin("Entity");
		std::vector<Entity> entities;
		entities.reserve(selectable_entities.size());
		for (const auto& handle : selectable_entities) {
			entities.emplace_back(scene, handle);
		}

		for (auto& entity : entities) {
			if (!entity.has_component<Components::Pipeline>())
				return;
			draw_component<Components::Pipeline>(entity, "Pipeline", [](Components::Pipeline& pipeline) {
				auto& [pipe] = pipeline;
				bool any_changed = false;
				{
					const auto preview_value = pipe->get_properties().depth_comparison_operator;
					auto [changed, new_or_old_value] = UI::combo_choice<DepthCompareOperator>("Compare operator", preview_value);
					any_changed |= changed;
					if (changed) {
						pipe->get_properties().depth_comparison_operator = new_or_old_value;
					}
				}
				{
					const auto preview_value = pipe->get_properties().cull_mode;
					auto [changed, new_or_old_value] = UI::combo_choice<CullMode>("Cull mode", preview_value);
					any_changed |= changed;
					if (changed) {
						pipe->get_properties().cull_mode = new_or_old_value;
					}
				}

				if (any_changed) {
					pipe->recreate(true);
				}
			});
		}
		UI::end();

		UI::end();
	} // namespace Disarray::Client

} // namespace Disarray::Client
