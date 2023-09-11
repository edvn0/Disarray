#include "panels/ExecutionStatisticsPanel.hpp"

namespace Disarray::Client {

using namespace std::string_view_literals;

void ExecutionStatisticsPanel::update(float time_step)
{
	if (!has_stats) {
		return;
	}

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

struct FloatFormatter {
	auto operator()(float float_value) const -> std::string { return fmt::format("{:.3f}", float_value); }
};
static constexpr const FloatFormatter formatter {};

void ExecutionStatisticsPanel::interface()
{
	if (!has_stats) {
		UI::scope("ExecutionStatisticsPanel"sv, []() { UI::text("{}", "No GPU stats collected."); });
	}

	UI::Tabular::table<float>("ExecutionStatisticsPanel"sv,
		{
			{ "GPU Time"sv, float(gpu_execution_time) },
			{ "IA Vertices"sv, float(input_assembly_vertices) },
			{ "IA Primitives"sv, float(input_assembly_primitives) },
			{ "VS Calls"sv, float(vertex_shader_invocations) },
			{ "Clip Calls"sv, float(clipping_invocations) },
			{ "Clip Primitives"sv, float(clipping_primitives) },
			{ "FS Calls"sv, float(fragment_shader_invocations) },
			{ "CS Calls"sv, float(compute_shader_invocations) },
		},
		formatter);
}

} // namespace Disarray::Client
