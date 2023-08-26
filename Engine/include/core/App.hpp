#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "core/Layer.hpp"
#include "core/events/Event.hpp"
#include "ui/InterfaceLayer.hpp"

namespace Disarray {

class Window;
class Device;
class Swapchain;
class PhysicalDevice;
class Renderer;

struct ApplicationProperties {
	std::uint32_t width { 0 };
	std::uint32_t height { 0 };
	std::string name {};
	bool is_fullscreen { false };
	std::filesystem::path working_directory { std::filesystem::current_path() };
};

/**
 * @brief Statistics for the app
 */
struct ApplicationStatistics {
	/** @brief (ms) Time for layer updates / rendering */
	double cpu_time { 0 };

	/** @brief (ms) Time for full frame */
	double frame_time { 0 };

	/** @brief (ns) Time for presentation */
	double presentation_time { 0 };
};

class App {
public:
	virtual ~App();
	explicit App(const ApplicationProperties&);
	void run();

	virtual void on_attach() = 0;
	virtual void on_detach() = 0;

	template <typename T, typename... Args>
	auto add_layer(Args&&... args) -> decltype(auto)
		requires(
			std::is_base_of_v<Layer, T> && requires(Disarray::Device& dev, Disarray::Window& win, Disarray::Swapchain& swap) { T(dev, win, swap); })
	{
		return layers.emplace_back(std::shared_ptr<T> { new T(*device, *window, *swapchain, std::forward<Args>(args)...) });
	}

	template <typename T, typename... Args>
		requires(std::is_base_of_v<Panel, T>
			&& requires(Disarray::Device& dev, Disarray::Window& win, Disarray::Swapchain& swap,
				Args&&... args) { T(dev, win, swap, std::forward<Args>(args)...); })
	auto add_panel(Args&&... args) -> auto&
	{
		std::shared_ptr<Layer> interface {
			nullptr
		};
		for (const auto& layer : layers) {
			if (layer->is_interface_layer()) {
				interface = layer;
				break;
			}
		}

		auto interface_layer = std::dynamic_pointer_cast<UI::InterfaceLayer>(interface);
		return interface_layer->template add_panel<T>(std::forward<Args>(args)...);
	}

	void on_event(Event& event);

	auto get_statistics() const -> const auto& { return statistics; }
	auto get_swapchain() const -> const auto& { return *swapchain; }

private:
	auto could_prepare_frame() -> bool;

	Scope<Window> window { nullptr };
	Scope<Device> device { nullptr };
	Scope<Swapchain> swapchain { nullptr };
	std::vector<std::shared_ptr<Layer>> layers {};
	ApplicationStatistics statistics;
};

extern auto create_application(const Disarray::ApplicationProperties&) -> std::unique_ptr<Disarray::App>;

} // namespace Disarray
