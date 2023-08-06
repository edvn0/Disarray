#include <Disarray.hpp>
#include <array>
#include <vector>

namespace Disarray::Client {

	class AppLayer : public Layer {
	public:
		AppLayer(Device& dev, Window& win, Swapchain& swap);
		~AppLayer() override;

		void interface() override;
		void construct(App& app, Renderer& renderer, ThreadPool&) override;
		void handle_swapchain_recreation(Renderer& renderer) override;
		void update(float ts) override;
		void update(float ts, Renderer& renderer) override;
		void destruct() override;

	private:
		Device& device;
		Window& window;
		Swapchain& swapchain;

		Ref<Pipeline> pipeline;
		Ref<Pipeline> geometry_pipeline;

		bool viewport_panel_mouse_over { false };
		bool viewport_panel_focused { false };
		std::array<glm::vec2, 2> viewport_bounds {};

		Ref<Mesh> viking_mesh;
		Ref<Texture> viking_room;

		Ref<Framebuffer> framebuffer;

		Ref<CommandExecutor> command_executor;
	};

} // namespace Disarray::Client
