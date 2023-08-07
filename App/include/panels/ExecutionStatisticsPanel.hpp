#pragma once

#include "MovingAverage.hpp"
#include "imgui.h"
#include "ui/UI.hpp"

#include <Disarray.hpp>
#include <array>
#include <concepts>

namespace Disarray::Client {

	using namespace std::string_view_literals;

	class ExecutionStatisticsPanel : public Panel {
		static constexpr auto update_interval_ms = 160;

	public:
		ExecutionStatisticsPanel(Device&, Window&, Swapchain& sc, const CommandExecutor& exec)
			: executor(exec)
			, swapchain(sc)
			, has_stats(exec.has_stats()) {

			};

		void update(float ts, Renderer& renderer) override
		{
			if (!has_stats)
				return;

			should_update_counter += ts;
			if (should_update_counter > update_interval_ms) {
				should_update_counter = 0;
				const PipelineStatistics& pipeline_stats = executor.get_pipeline_statistics(swapchain.get_current_frame());
				gpu_execution_time(executor.get_gpu_execution_time(swapchain.get_current_frame()));
				input_assembly_vertices(pipeline_stats.input_assembly_vertices);
				input_assembly_primitives(pipeline_stats.input_assembly_primitives);
				vertex_shader_invocations(pipeline_stats.vertex_shader_invocations);
				clipping_invocations(pipeline_stats.clipping_invocations);
				clipping_primitives(pipeline_stats.clipping_primitives);
				fragment_shader_invocations(pipeline_stats.fragment_shader_invocations);
				compute_shader_invocations(pipeline_stats.compute_shader_invocations);
			}
		}

		void interface() override
		{
			if (!has_stats) {
				UI::scope("ExecutionStatisticsPanel"sv, []() { ImGui::Text("No GPU stats collected."); });
			}

			UI::scope("ExecutionStatisticsPanel"sv, [&]() {
				if (ImGui::BeginTable("StatisticsTable", 2)) {
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "GPU time");
						ImGui::TableNextColumn();
						ImGui::Text("%fms", double(gpu_execution_time));
					}
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "IA Vertices");
						ImGui::TableNextColumn();
						ImGui::Text("%f", double(input_assembly_vertices));
					}
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "IA Primitives");
						ImGui::TableNextColumn();
						ImGui::Text("%f", double(input_assembly_primitives));
					}
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "VS Calls");
						ImGui::TableNextColumn();
						ImGui::Text("%f", double(vertex_shader_invocations));
					}
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "Clip Calls");
						ImGui::TableNextColumn();
						ImGui::Text("%f", double(clipping_invocations));
					}
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "Clip Primitives");
						ImGui::TableNextColumn();
						ImGui::Text("%f", double(clipping_primitives));
					}
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "FS Calls");
						ImGui::TableNextColumn();
						ImGui::Text("%f", double(fragment_shader_invocations));
					}
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "CS Calls");
						ImGui::TableNextColumn();
						ImGui::Text("%f", double(compute_shader_invocations));
					}
					ImGui::EndTable();
				}
			});
		}

	private:
		const CommandExecutor& executor;
		Swapchain& swapchain;
		bool has_stats { true };
		double should_update_counter { 0.0 };

		static constexpr std::size_t keep_frames = 144;

		MovingAverage<float, float, keep_frames> gpu_execution_time;
		MovingAverage<float, float, keep_frames> input_assembly_vertices;
		MovingAverage<float, float, keep_frames> input_assembly_primitives;
		MovingAverage<float, float, keep_frames> vertex_shader_invocations;
		MovingAverage<float, float, keep_frames> clipping_invocations;
		MovingAverage<float, float, keep_frames> clipping_primitives;
		MovingAverage<float, float, keep_frames> fragment_shader_invocations;
		MovingAverage<float, float, keep_frames> compute_shader_invocations;
	};

} // namespace Disarray::Client
