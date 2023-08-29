#include "panels/ExecutionStatisticsPanel.hpp"

namespace Disarray::Client {

void ExecutionStatisticsPanel::update(float time_step)
{
	if (!has_stats)
		return;

	should_update_counter += time_step;
	if (should_update_counter > update_interval_ms) {
		should_update_counter = 0;
		const PipelineStatistics& pipeline_stats = executor.get_pipeline_statistics(swapchain.get_current_frame());
		gpu_execution_time(executor.get_gpu_execution_time(swapchain.get_current_frame()));
		input_assembly_vertices(static_cast<float>(pipeline_stats.input_assembly_vertices));
		input_assembly_primitives(static_cast<float>(pipeline_stats.input_assembly_primitives));
		vertex_shader_invocations(static_cast<float>(pipeline_stats.vertex_shader_invocations));
		clipping_invocations(static_cast<float>(pipeline_stats.clipping_invocations));
		clipping_primitives(static_cast<float>(pipeline_stats.clipping_primitives));
		fragment_shader_invocations(static_cast<float>(pipeline_stats.fragment_shader_invocations));
		compute_shader_invocations(static_cast<float>(pipeline_stats.compute_shader_invocations));
	}
}

void ExecutionStatisticsPanel::interface()
{
	if (!has_stats) {
		UI::scope("ExecutionStatisticsPanel"sv, []() { ImGui::Text("No GPU stats collected."); });
	}

	UI::scope("ExecutionStatisticsPanel"sv, [&]() {
		if (ImGui::BeginTable("StatisticsTable", 2)) {
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				UI::text("{}", "GPU time");
				ImGui::TableNextColumn();
				UI::text("{}", float(gpu_execution_time));
			}
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				UI::text("{}", "IA Vertices");
				ImGui::TableNextColumn();
				UI::text("{}", float(input_assembly_vertices));
			}
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				UI::text("{}", "IA Primitives");
				ImGui::TableNextColumn();
				UI::text("{}", float(input_assembly_primitives));
			}
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				UI::text("{}", "VS Calls");
				ImGui::TableNextColumn();
				UI::text("{}", float(vertex_shader_invocations));
			}
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				UI::text("{}", "Clip Calls");
				ImGui::TableNextColumn();
				UI::text("{}", float(clipping_invocations));
			}
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				UI::text("{}", "Clip Primitives");
				ImGui::TableNextColumn();
				UI::text("{}", float(clipping_primitives));
			}
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				UI::text("{}", "FS Calls");
				ImGui::TableNextColumn();
				UI::text("{}", float(fragment_shader_invocations));
			}
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				UI::text("{}", "CS Calls");
				ImGui::TableNextColumn();
				UI::text("{}", float(compute_shader_invocations));
			}
			ImGui::EndTable();
		}
	});
}

} // namespace Disarray::Client
