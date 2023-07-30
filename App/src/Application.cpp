
#include "core/AllocatorConfigurator.hpp"
#include "graphics/ImageProperties.hpp"
#include "graphics/PushContantLayout.hpp"
#include "graphics/Texture.hpp"

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

	void construct(Ref<Renderer> renderer) override
	{
		VertexLayout layout { {
			{ ElementType::Float3, "position" },
			{ ElementType::Float2, "uv" },
			{ ElementType::Float4, "colour" },
			{ ElementType::Float3, "normals" },
		} };

		render_pass = RenderPass::construct(device, { .image_format = Disarray::ImageFormat::SBGR });

		auto extent = swapchain->get_extent();

		const auto& [vert, frag] = renderer->get_pipeline_cache().get_shader("main");
		const PipelineProperties props = {
			.vertex_shader = vert,
			.fragment_shader = frag,
			.render_pass = render_pass,
			.layout = layout,
			.push_constant_layout = PushConstantLayout { PushConstantRange { PushConstantKind::Both, std::size_t { 80 } } },
			.extent = { extent.width, extent.height },
		};
		pipeline = Pipeline::construct(device, swapchain, props);
		framebuffer = Framebuffer::construct(device, swapchain, physical_device, render_pass, {});
		command_executor = CommandExecutor::construct_from_swapchain(device, swapchain, physical_device->get_queue_family_indexes(), { .count = 2 });
		viking_mesh = Mesh::construct(device, swapchain, physical_device, { .path = "Assets/Models/viking.mesh", .pipeline = pipeline });

#define IS_TESTING
#ifdef IS_TESTING
		{
			std::uint32_t white_tex[] = { 0 };
			auto pixels = DataBuffer(white_tex, sizeof(std::uint32_t));
			TextureProperties texture_properties {
				.extent = swapchain->get_extent(),
				.format = ImageFormat::SBGR,
			};
			Ref<Texture> tex = Texture::construct(device, swapchain, physical_device, texture_properties);
			texture_properties.path = "Assets/Textures/viking_room.png";
			viking_room = Texture::construct(device, swapchain, physical_device, texture_properties);
		};

		Log::debug("Constructed AppLayer.");
#endif
#undef IS_TESTING
	};

	void handle_swapchain_recreation(Ref<Renderer> renderer) override
	{
		renderer->set_extent(swapchain->get_extent());
		framebuffer->force_recreation();
		render_pass->force_recreation();
		pipeline->force_recreation();
		command_executor->force_recreation();
		viking_room->force_recreation();
	}

	void update(float ts) override {

	};

	void update(float ts, Ref<Renderer> renderer) override
	{
		command_executor->begin();
		renderer->begin_pass(command_executor, render_pass, framebuffer);
		// const auto&& [mid_x, mid_y] = renderer->center_position();
		renderer->draw_mesh(command_executor, viking_mesh);
		renderer->draw_planar_geometry(Geometry::Rectangle, { .position = { 0, 0, 0 }, .dimensions = { { 12.f, 12.f, 1.f } } });
		// renderer->draw_text("Hello world!", 0, 0, 12.f);
		// renderer->draw_geometry(Geometry::Circle, { .pos = glm::vec3(mid_x, mid_y), .size = 12 });
		renderer->end_pass(command_executor);
		command_executor->submit_and_end();
	}

	void destruct() override { Log::debug("Destructed AppLayer."); };

private:
	Ref<Device> device;
	Ref<PhysicalDevice> physical_device;
	Scope<Window>& window;
	Ref<Swapchain> swapchain;

	Ref<Pipeline> pipeline;
	Ref<Framebuffer> framebuffer;
	Ref<RenderPass> render_pass;
	Ref<CommandExecutor> command_executor;

	Ref<Mesh> viking_mesh;
	Ref<Texture> viking_room;

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