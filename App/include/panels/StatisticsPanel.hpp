#pragma once

#include "MovingAverage.hpp"
#include "imgui.h"
#include "ui/UI.hpp"

#include <Disarray.hpp>
#include <array>
#include <concepts>

namespace Disarray::Client {

	class StatisticsPanel : public Panel {
		static constexpr auto update_interval_ms = 30.0;

	public:
		StatisticsPanel(Device&, Window&, Swapchain&, const ApplicationStatistics& stats)
			: statistics(stats) {};

		void update(float ts, Renderer& renderer) override
		{
			should_update_counter += ts;
			if (should_update_counter > update_interval_ms) {
				should_update_counter = 0;
				const auto& [cpu_time, frame_time, presentation_time] = statistics;
				cpu_time_average(cpu_time);
				frame_time_average(frame_time);
				presentation_time_average(presentation_time);
			}
		}

		void interface() override
		{
			using namespace std::string_view_literals;
			UI::scope("StatisticsPanel"sv, [&]() {
				if (ImGui::BeginTable("StatisticsTable", 2)) {
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "Frametime");
						ImGui::TableNextColumn();
						ImGui::Text("%fms", double(frame_time_average));
					}
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "CPU");
						ImGui::TableNextColumn();
						ImGui::Text("%fms", double(cpu_time_average));
					}
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "FPS");
						ImGui::TableNextColumn();
						ImGui::Text("%fms", double(1000.0 * frame_time_average.inverse()));
					}
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s", "Presentation");
						ImGui::TableNextColumn();
						ImGui::Text("%fus", double(presentation_time_average));
					}
					ImGui::EndTable();
				}
			});
		}

	private:
		double should_update_counter { 0.0 };
		const ApplicationStatistics& statistics;

		MovingAverage<double, double, (144 * 6) / 30> frame_time_average;
		MovingAverage<double, double, (144 * 6) / 30> cpu_time_average;
		MovingAverage<double, double, (144 * 6) / 30> presentation_time_average;
	};

} // namespace Disarray::Client
