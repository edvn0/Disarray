#pragma once

#include <Disarray.hpp>
#include <imgui.h>

#include <array>
#include <concepts>

#include "MovingAverage.hpp"
#include "ui/UI.hpp"

namespace Disarray::Client {

class StatisticsPanel : public Panel {
	static constexpr auto update_interval_ms = 30.0;

public:
	StatisticsPanel(Device&, Window&, Swapchain&, const ApplicationStatistics& stats)
		: statistics(stats) {};

	void update(float time_step) override
	{
		should_update_counter += time_step;
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
					UI::text("{}", "Frametime");
					ImGui::TableNextColumn();
					UI::text("{}ms", double(frame_time_average));
				}
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					UI::text("{}", "CPU");
					ImGui::TableNextColumn();
					UI::text("{}ms", double(cpu_time_average));
				}
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					UI::text("{}", "FPS");
					ImGui::TableNextColumn();
					UI::text("{}ms", double(1000.0 * frame_time_average.inverse()));
				}
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					UI::text("{}", "Presentation");
					ImGui::TableNextColumn();
					UI::text("{}us", double(presentation_time_average));
				}
				ImGui::EndTable();
			}
		});
	}

private:
	double should_update_counter { 0.0 };
	const ApplicationStatistics& statistics;

	static constexpr auto frame_keep = (144 * 6) / 30;
	MovingAverage<double, double, frame_keep> frame_time_average;
	MovingAverage<double, double, frame_keep> cpu_time_average;
	MovingAverage<double, double, frame_keep> presentation_time_average;
};

} // namespace Disarray::Client
