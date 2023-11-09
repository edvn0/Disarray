#include "panels/StatisticsPanel.hpp"

namespace Disarray::Client {

StatisticsPanel::StatisticsPanel(Device&, Window&, Swapchain&, const ApplicationStatistics& stats)
	: statistics(stats) {};

void StatisticsPanel::update(float time_step)
{
	const auto& [cpu_time, frame_time, presentation_time] = statistics;
	cpu_time_average(cpu_time);
	frame_time_average(frame_time);
	presentation_time_average(presentation_time);
}

void StatisticsPanel::interface()
{
	using namespace std::string_view_literals;
	UI::scope("StatisticsPanel"sv, [this]() {
		if (ImGui::BeginTable("StatisticsTable", 2)) {
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				UI::text("{}", "Frametime");
				ImGui::TableNextColumn();
				UI::text("{:.3f}ms", double(frame_time_average));
			}
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				UI::text("{}", "CPU");
				ImGui::TableNextColumn();
				UI::text("{:.3f}ms", double(cpu_time_average));
			}
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				UI::text("{}", "FPS");
				ImGui::TableNextColumn();
				UI::text("{:.3f}", double(1000.0 * frame_time_average.inverse()));
			}
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				UI::text("{}", "Presentation");
				ImGui::TableNextColumn();
				UI::text("{:.3f}us", double(presentation_time_average));
			}
			ImGui::EndTable();
		}
	});
}

} // namespace Disarray::Client
