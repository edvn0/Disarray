#pragma once

#include <Disarray.hpp>

#include <array>
#include <concepts>

#include "ui/UI.hpp"

namespace Disarray::Client {

class PipelineEditorPanel : public Panel {
	static constexpr auto update_interval_ms = 30.0;

public:
	PipelineEditorPanel(Device&, Window&, Swapchain&, PipelineCache& cache);
	void update(float time_step) override;
	void interface() override;

private:
	PipelineCache& pipeline_cache;
	const Disarray::Device& device;
	struct PreviewValue {
		Ref<Disarray::Pipeline> preview_value = nullptr;
		std::string name {};
	};
	PreviewValue preview {};
};

} // namespace Disarray::Client
