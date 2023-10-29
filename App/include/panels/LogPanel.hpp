#pragma once

#include <Disarray.hpp>

#include <array>
#include <concepts>

#include "ui/UI.hpp"

namespace Disarray::Client {

class LogPanel : public Panel {
	static constexpr auto update_interval_ms = 30.0;

public:
	LogPanel(Device&, Window&, Swapchain&, std::filesystem::path log_file);
	void update(float time_step) override;
	void interface() override;

private:
	std::ifstream file_stream {};
};

} // namespace Disarray::Client
