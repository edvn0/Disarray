#pragma once

#include "Forward.hpp"

#include <queue>
#include <vector>

#include "core/Collections.hpp"
#include "core/Layer.hpp"
#include "core/Panel.hpp"
#include "core/UsageBadge.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Renderer.hpp"
#include "imgui_internal.h"

namespace Disarray::UI {

class InterfaceLayer : public Disarray::Layer {
public:
	InterfaceLayer(Disarray::Device& dev, Disarray::Window& win, Disarray::Swapchain& swap);
	~InterfaceLayer() override;

	void construct(App&) override;
	void handle_swapchain_recreation(Swapchain&) override;
	void on_event(Event&) override;
	void interface() override;
	void update(float time_step) override;
	void destruct() override;
	void render() override;
	auto is_interface_layer() const -> bool override { return true; }

	template <typename T, typename... Args>
		requires(std::is_base_of_v<Panel, T>
			&& requires(Disarray::Device& dev, Disarray::Window& win, Disarray::Swapchain& swap,
				Args&&... args) { T(dev, win, swap, std::forward<Args>(args)...); })
	auto add_panel(Args&&... args) -> auto&
	{
		return panels.emplace_back(std::shared_ptr<T> { new T { device, window, swapchain, std::forward<Args>(args)... } });
	}

	template <class Func>
		requires requires(Func f, CommandExecutor& exec) {
			{
				f.operator()(exec)
			} -> std::same_as<void>;
		}
	static void on_frame_end(Func&& func)
	{
		frame_end_callbacks.push(std::forward<Func>(func));
	}

	void begin();
	void end();

private:
	std::vector<std::shared_ptr<Panel>> panels {};
	Device& device;
	Window& window;
	Swapchain& swapchain;

	struct RendererSpecific;
	std::unique_ptr<RendererSpecific> pimpl { nullptr };

	Ref<Disarray::CommandExecutor> command_executor;

	using FrameEndCallback = std::function<void(CommandExecutor&)>;
	static inline std::queue<FrameEndCallback> frame_end_callbacks {};
};

} // namespace Disarray::UI
