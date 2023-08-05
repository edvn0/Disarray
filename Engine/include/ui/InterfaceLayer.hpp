#pragma once

#include "Forward.hpp"
#include "core/Layer.hpp"
#include "core/Panel.hpp"
#include "graphics/CommandExecutor.hpp"

#include <vector>

namespace Disarray::UI {

	class InterfaceLayer : public Disarray::Layer {
	public:
		InterfaceLayer(Disarray::Device& dev, Disarray::Window& win, Disarray::Swapchain& swap);
		~InterfaceLayer() override;

		void construct(App&, Renderer&, ThreadPool&) override;
		void handle_swapchain_recreation(Renderer&) override;

		void interface() override;

		void update(float ts) override;
		void update(float ts, Renderer&) override;

		void destruct() override;

		bool is_interface_layer() const override { return true; }

		template <typename T, typename... Args>
			requires(std::is_base_of_v<Panel, T>
				&& requires(Disarray::Device& dev, Disarray::Window& win, Disarray::Swapchain& swap,
					Args&&... args) { T(dev, win, swap, std::forward<Args>(args)...); })
		void add_panel(Args&&... args)
		{
			panels.emplace_back(std::shared_ptr<T> { new T { device, window, swapchain, std::forward<Args>(args)... } });
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
	};

} // namespace Disarray::UI
