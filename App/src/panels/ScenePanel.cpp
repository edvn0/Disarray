#include "panels/ScenePanel.hpp"

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

		static constexpr auto convert = [](DepthCompareOperator op) -> std::string_view {
			using namespace std::string_view_literals;
			switch (op) {
			case DepthCompareOperator::None:
				return "DepthCompareOperator::None"sv;
			case DepthCompareOperator::Never:
				return "DepthCompareOperator::Never"sv;
			case DepthCompareOperator::NotEqual:
				return "DepthCompareOperator::NotEqual"sv;
			case DepthCompareOperator::Less:
				return "DepthCompareOperator::Less"sv;
			case DepthCompareOperator::LessOrEqual:
				return "DepthCompareOperator::LessOrEqual"sv;
			case DepthCompareOperator::Greater:
				return "DepthCompareOperator::Greater"sv;
			case DepthCompareOperator::GreaterOrEqual:
				return "DepthCompareOperator::GreaterOrEqual"sv;
			case DepthCompareOperator::Equal:
				return "DepthCompareOperator::Equal"sv;
			case DepthCompareOperator::Always:
				return "DepthCompareOperator::Always"sv;
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
			std::uint32_t item_current_idx = get_index(operators, preview_value);
			if (ImGui::BeginCombo("Compare operator", convert(preview_value).data())) {
				for (std::uint32_t n = 0; n < operators.size(); n++) {
					const bool is_selected = (item_current_idx == n);
					if (ImGui::Selectable(convert(operators[n]).data(), is_selected))
						item_current_idx = n;

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			if (preview_value != operators[item_current_idx]) {
				pipe.pipeline->get_properties().depth_comparison_operator = operators[item_current_idx];
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
