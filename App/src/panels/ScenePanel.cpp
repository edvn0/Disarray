#include "panels/ScenePanel.hpp"

#include "graphics/Pipeline.hpp"

namespace Disarray::Client {

	void ScenePanel::interface()
	{
		auto& registry = scene.get_registry();
		static bool debug_log_window_open { true };
		static bool window_open { true };
		ImGui::ShowDebugLogWindow(&debug_log_window_open);
		ImGui::ShowDemoWindow(&window_open);

		UI::begin("Scene");
		auto pipelines_and_meshes = registry.view<Components::Pipeline, Components::Mesh>();
		std::array<DepthCompareOperator, 9> operators {
			DepthCompareOperator::None,
			DepthCompareOperator::Never,
			DepthCompareOperator::NotEqual,
			DepthCompareOperator::Less,
			DepthCompareOperator::LessOrEqual,
			DepthCompareOperator::Greater,
			DepthCompareOperator::GreaterOrEqual,
			DepthCompareOperator::Equal,
			DepthCompareOperator::Always,
		};

		static constexpr const auto to_string = [](DepthCompareOperator op) -> std::string {
			switch (op) {
			case DepthCompareOperator::None:
				return "DepthCompareOperator::None";
			case DepthCompareOperator::Never:
				return "DepthCompareOperator::Never";
			case DepthCompareOperator::NotEqual:
				return "DepthCompareOperator::NotEqual";
			case DepthCompareOperator::Less:
				return "DepthCompareOperator::Less";
			case DepthCompareOperator::LessOrEqual:
				return "DepthCompareOperator::LessOrEqual";
			case DepthCompareOperator::Greater:
				return "DepthCompareOperator::Greater";
			case DepthCompareOperator::GreaterOrEqual:
				return "DepthCompareOperator::GreaterOrEqual";
			case DepthCompareOperator::Equal:
				return "DepthCompareOperator::Equal";
			case DepthCompareOperator::Always:
				return "DepthCompareOperator::Always";
			default:
				unreachable();
			}
		};

		static constexpr auto get_index = [](const auto& operators, auto op) -> std::uint32_t {
			std::uint32_t i = 0;
			for (const auto& oper : operators) {
				if (op == oper)
					return i;
				i++;
			}
			unreachable();
		};

		for (const auto [entity, pipe, mesh] : pipelines_and_meshes.each()) {
			const auto preview_value = pipe.pipeline->get_properties().depth_comparison_operator;
			auto [changed, new_or_old_value] = UI::combo_choice<DepthCompareOperator>("Compare operator", preview_value);
			if (changed) {
				pipe.pipeline->get_properties().depth_comparison_operator = new_or_old_value;
				pipe.pipeline->recreate(true);
			}
		}
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
				ImGui::Text("%lu", id.identifier);
			}

			ImGui::EndTable();
		}
		UI::end();
	} // namespace Disarray::Client

} // namespace Disarray::Client
