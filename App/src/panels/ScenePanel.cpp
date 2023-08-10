#include "panels/ScenePanel.hpp"

namespace Disarray::Client {

	void ScenePanel::interface()
	{
		static bool debug_log_window_open { true };
		static bool window_open { true };
		ImGui::ShowDebugLogWindow(&debug_log_window_open);
		ImGui::ShowDemoWindow(&window_open);

		UI::begin("Scene");
		auto& registry = scene.get_registry();
		if (ImGui::BeginTable("StatisticsTable", 2)) {
			const auto inheritance_view = registry.view<const ID, const Tag, const Inheritance>();
			for (const auto& [entity, id, tag, inheritance] : inheritance_view.each()) {
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
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("%s", tag.name.c_str());
				ImGui::TableNextColumn();
				ImGui::Text("%llu", id.identifier);
			}

			ImGui::EndTable();
		}
		UI::end();
	} // namespace Disarray::Client

} // namespace Disarray::Client
