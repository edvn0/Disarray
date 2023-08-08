#include <Disarray.hpp>
#include <array>
#include <vector>

namespace Disarray::Client {

	class AppLayer : public Layer {
	public:
		AppLayer(Device&, Window&, Swapchain&);
		~AppLayer() override;

		void interface() override;

		void construct(App& app, Renderer& renderer, ThreadPool&) override;
		void handle_swapchain_recreation(Renderer& renderer) override;
		void on_event(Event&) override;
		void update(float ts) override;
		void update(float ts, Renderer& renderer) override;
		void destruct() override;

	private:
		Window& window;
		Swapchain& swapchain;
		Scene scene;

		bool viewport_panel_mouse_over { false };
		bool viewport_panel_focused { false };
		std::array<glm::vec2, 2> viewport_bounds {};
	};

} // namespace Disarray::Client
