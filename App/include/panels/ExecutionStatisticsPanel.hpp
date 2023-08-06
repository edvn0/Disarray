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
		static constexpr auto update_interval_ms = 0.1;

	public:
		ExecutionStatisticsPanel(Device&, Window&, Swapchain& sc, const CommandExecutor& exec)
			: executor(exec)
			, swapchain(sc)
			, has_stats(exec.has_stats()) {

			};

		void update(float ts, Renderer& renderer) override { }

		void interface() override
		{
			if (!has_stats)
				return;

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
						ImGui::Text("%s", "IA Vertices");
						ImGui::TableNextColumn();
						ImGui::Text("%lu", pipeline_stats.input_assembly_vertices);
					}
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "IA Primitives");
						ImGui::TableNextColumn();
						ImGui::Text("%lu", pipeline_stats.input_assembly_primitives);
					}
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "VS Calls");
						ImGui::TableNextColumn();
						ImGui::Text("%lu", pipeline_stats.vertex_shader_invocations);
					}
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "Clip Calls");
						ImGui::TableNextColumn();
						ImGui::Text("%lu", pipeline_stats.clipping_invocations);
					}
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "Clip Primitives");
						ImGui::TableNextColumn();
						ImGui::Text("%lu", pipeline_stats.clipping_primitives);
					}
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "FS Calls");
						ImGui::TableNextColumn();
						ImGui::Text("%lu", pipeline_stats.fragment_shader_invocations);
					}
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "CS Calls");
						ImGui::TableNextColumn();
						ImGui::Text("%lu", pipeline_stats.compute_shader_invocations);
					}
					ImGui::EndTable();
				}
			});
		}

	private:
		const CommandExecutor& executor;
		Swapchain& swapchain;
		bool has_stats { true };
	};

} // namespace Disarray::Client
