#include <Disarray.hpp>
#include <array>
#include <vector>

namespace Disarray::Client {

	class ClientLayer : public Layer {
	public:
		ClientLayer(Device&, Window&, Swapchain&);
		~ClientLayer() override;

		void interface() override;

		void construct(App&, Renderer&, ThreadPool&) override;
		void handle_swapchain_recreation(Swapchain&) override;
		void on_event(Event&) override;
		void update(float) override;
		void render(Renderer&) override;
		void destruct() override;

	private:
		Scope<Scene> scene;
		Device& device;

		EditorCamera camera;

		bool viewport_panel_mouse_over { false };
		bool viewport_panel_focused { false };
		std::array<glm::vec2, 2> viewport_bounds {};
	};

} // namespace Disarray::Client
