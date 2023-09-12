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
	StatisticsPanel(Device&, Window&, Swapchain&, const ApplicationStatistics& stats);
	void update(float time_step) override;
	void interface() override;

private:
	double should_update_counter { 0.0 };
	const ApplicationStatistics& statistics;

	static constexpr auto frame_keep = (144 * 6) / 30;
	MovingAverage<double, double, frame_keep> frame_time_average;
	MovingAverage<double, double, frame_keep> cpu_time_average;
	MovingAverage<double, double, frame_keep> presentation_time_average;
};

} // namespace Disarray::Client
