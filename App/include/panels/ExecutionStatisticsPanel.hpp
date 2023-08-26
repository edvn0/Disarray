#pragma once

#include <Disarray.hpp>
#include <imgui.h>

#include <array>
#include <concepts>

#include "MovingAverage.hpp"
#include "ui/UI.hpp"

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

	void update(float ts) override;

	void interface() override;

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
