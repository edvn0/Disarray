#include <Disarray.hpp>
#include <vector>

namespace Disarray::Client {

	using namespace Disarray;

	class AppLayer : public Layer {
	public:
		AppLayer(Device& dev, Window& win, Swapchain& swap);
		~AppLayer() override;

		void interface() override;
		void construct(App& app, Renderer& renderer) override;
		void handle_swapchain_recreation(Renderer& renderer);
		void update(float ts) override;
		void update(float ts, Renderer& renderer) override;
		void destruct() override;

	private:
		Device& device;
		Window& window;
		Swapchain& swapchain;

		Ref<Pipeline> pipeline;
		Ref<Pipeline> geometry_pipeline;

		Ref<Mesh> viking_mesh;
		Ref<Texture> viking_room;

		Ref<Framebuffer> framebuffer;
		Ref<Framebuffer> second_framebuffer;

		Ref<CommandExecutor> command_executor;
	};

} // namespace Disarray::Client
