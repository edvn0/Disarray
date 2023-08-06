#pragma once

#include "MovingAverage.hpp"
#include "imgui.h"
#include "ui/UI.hpp"

#include <Disarray.hpp>
#include <array>
#include <concepts>

namespace Disarray::Client {

	class ExecutionStatisticsPanel : public Panel {
		static constexpr auto update_interval_ms = 0.1;

	public:
		ExecutionStatisticsPanel(Device&, Window&, Swapchain& sc, const CommandExecutor& exec)
			: executor(exec)
			, swapchain(sc) {};

		void update(float ts, Renderer& renderer) override { }

		void interface() override
		{
			using namespace std::string_view_literals;
			UI::scope("ExecutionStatisticsPanel"sv, [&]() {
				if (ImGui::BeginTable("StatisticsTable", 2)) {
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "GPU time");
						ImGui::TableNextColumn();
						ImGui::Text("%fms", double(executor.get_gpu_execution_time(swapchain.get_current_frame())));
					}
					const PipelineStatistics& pipeline_stats = executor.get_pipeline_statistics(swapchain.get_current_frame());
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "Input Assembly Vertices");
						ImGui::TableNextColumn();
						ImGui::Text("%llu", pipeline_stats.input_assembly_vertices);
					}
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "Input Assembly Primitives");
						ImGui::TableNextColumn();
						ImGui::Text("%llu", pipeline_stats.input_assembly_primitives);
					}
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "Vertex Shader Calls");
						ImGui::TableNextColumn();
						ImGui::Text("%llu", pipeline_stats.vertex_shader_invocations);
					}
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "Clipping Calls");
						ImGui::TableNextColumn();
						ImGui::Text("%llu", pipeline_stats.clipping_invocations);
					}
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "Clipping Primitives");
						ImGui::TableNextColumn();
						ImGui::Text("%llu", pipeline_stats.clipping_primitives);
					}
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "Fragment Shader Calls");
						ImGui::TableNextColumn();
						ImGui::Text("%llu", pipeline_stats.fragment_shader_invocations);
					}
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "Compute Shader Calls");
						ImGui::TableNextColumn();
						ImGui::Text("%llu", pipeline_stats.compute_shader_invocations);
					}
					ImGui::EndTable();
				}
			});
		}

	private:
		double should_update_counter { 0.0 };
		Swapchain& swapchain;
		const CommandExecutor& executor;
	};

} // namespace Disarray::Client
