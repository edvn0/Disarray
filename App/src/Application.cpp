#include <Disarray.hpp>
#include <vector>

using namespace Disarray;

class AppLayer : public Layer {
public:
	AppLayer(Ref<Device> dev, Ref<PhysicalDevice> phy, Scope<Window>& win, Ref<Swapchain> swap)
		: device(dev)
		, physical_device(phy)
		, window(win)
		, swapchain(swap) {

		};

	void construct() override
	{
		VertexLayout layout {
			{ ElementType::Float3 },
			{ ElementType::Float2 },
			{ ElementType::Float4 },
		};

		render_pass = RenderPass::construct(device, { .image_format = Disarray::ImageFormat::SBGR });

		auto extent = swapchain->get_extent();

		const PipelineProperties props = {
			.vertex_shader = Shader::construct(device,
				{
					.path = "assets/Shaders/main.vert.spv",
					.type = ShaderType::Vertex,
				}),
			.fragment_shader = Shader::construct(device,
				{
					.path = "assets/Shaders/main.frag.spv",
					.type = ShaderType::Fragment,
				}),
			.render_pass = render_pass,
			.layout = layout,
			.extent = { extent.width, extent.height },
		};
		pipeline = Pipeline::construct(device, swapchain, props);
		framebuffer = Framebuffer::construct(device, swapchain, render_pass, {});
		command_executor = CommandExecutor::construct_from_swapchain(device, physical_device, swapchain, window->get_surface(), { .count = 2 });

		Log::debug("Constructed AppLayer.");
	};

	void handle_swapchain_recreation(Ref<Renderer> renderer) override {
		framebuffer->force_recreation();
		render_pass->force_recreation();
		pipeline->force_recreation();
		command_executor->force_recreation();
		renderer->set_extent(swapchain->get_extent());
	}

	void update(float ts) override {

	};

	void update(float ts, Ref<Renderer> renderer) override
	{
		command_executor->begin();
		renderer->begin_pass(command_executor, render_pass, pipeline, framebuffer);
		// const auto&& [mid_x, mid_y] = renderer->center_position();
		// renderer->draw_mesh(my_mesh);
		renderer->draw_planar_geometry(command_executor, Geometry::Triangle, { .position = { 0, 0, 0 }, .dimensions = { { 12.f, 12.f, 1.f } } });
		// renderer->draw_text("Hello world!", 0, 0, 12.f);
		// renderer->draw_geometry(Geometry::Circle, { .pos = glm::vec3(mid_x, mid_y), .size = 12 });
		renderer->end_pass(command_executor);
		command_executor->submit_and_end();
	}

	void destruct() override { Log::debug("Destructed AppLayer."); };

private:
	Ref<Pipeline> pipeline;
	Ref<Framebuffer> framebuffer;
	Ref<RenderPass> render_pass;
	Ref<CommandExecutor> command_executor;

	Ref<Swapchain> swapchain;
	Scope<Window>& window;
	Ref<PhysicalDevice> physical_device;
	Ref<Device> device;
};

int main(int argc, char** argv)
{
	struct A { };

	std::vector<std::string_view> args(argv, argv + argc);

	using namespace Disarray;
	App app;
	app.add_layer<AppLayer>();
	app.run();
}