#pragma once

#include "core/Layer.hpp"

#include "Forward.hpp"

#include <vector>

namespace Disarray::UI {

	class InterfaceLayer : public Disarray::Layer {
	public:
		InterfaceLayer(Ref<Device> dev, Ref<PhysicalDevice> phy, Scope<Window>& win, Ref<Swapchain> swap);
		~InterfaceLayer() override;

		void construct(App&, Renderer&) override;
		void handle_swapchain_recreation(Renderer&) override;

		void update(float ts) override;
		void update(float ts, Renderer&) override;

		void destruct() override;

		bool is_interface_layer() const override {return true;}

		template<typename T, typename... Args> requires (std::is_base_of_v<Panel, T>)
		void add_panel(Args&&... args) {
			panels.emplace_back(Ref<T> {new T {std::forward(args)...}});
		}

		void begin();
		void end(Renderer&);

	private:
		std::vector<Ref<Panel>> panels {};
		Ref<Device> device;
		Ref<PhysicalDevice> physical_device;
		Scope<Window>& window;
		Ref<Swapchain> swapchain;
		Ref<Disarray::RenderPass> render_pass;
	};

}
