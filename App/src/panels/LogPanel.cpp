#include "panels/LogPanel.hpp"

namespace Disarray::Client {

LogPanel::LogPanel(Device&, Window&, Swapchain&, std::filesystem::path log_file)
	: file_stream(log_file)
{
	if (!file_stream)
		throw;
}

void LogPanel::update(float time_step) { }

void LogPanel::interface()
{
	using namespace std::string_view_literals;
	UI::scope("LogPanel"sv, [&]() {
		std::stringstream buffer;
		buffer << file_stream.rdbuf();
		std::string logContent = buffer.str();
		ImGui::InputTextMultiline("Log", logContent.data(), logContent.size(), ImGui::GetContentRegionAvail(), ImGuiInputTextFlags_ReadOnly);
	});
}

} // namespace Disarray::Client
