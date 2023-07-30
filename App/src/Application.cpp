
#include "core/AllocatorConfigurator.hpp"
#include "core/Layer.hpp"
#include "graphics/ImageProperties.hpp"
#include "graphics/PushContantLayout.hpp"
#include "graphics/Renderer.hpp"
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

	void construct(App& app, Renderer& renderer) override
	{
		VertexLayout layout { {
			{ ElementType::Float3, "position" },
			{ ElementType::Float2, "uv" },
			{ ElementType::Float4, "colour" },
			{ ElementType::Float3, "normals" },
		} };

		auto render_pass = RenderPass::construct(device, { .image_format = Disarray::ImageFormat::SBGR });

		auto extent = swapchain->get_extent();

		const auto& [vert, frag] = renderer.get_pipeline_cache().get_shader("main");
		const PipelineProperties props = {
			.vertex_shader = vert,
			.fragment_shader = frag,
			.render_pass = render_pass,
			.layout = layout,
			.push_constant_layout = PushConstantLayout { PushConstantRange { PushConstantKind::Both, std::size_t { 80 } } },
			.extent = { extent.width, extent.height },
		};
		pipeline = Pipeline::construct(device, swapchain, props);
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

	void handle_swapchain_recreation(Renderer& renderer) override
	{
		renderer.set_extent(swapchain->get_extent());
		pipeline->force_recreation();
		viking_room->force_recreation();
	}

	void update(float ts) override {

	};

	void update(float ts, Renderer& renderer) override
	{
		// const auto&& [mid_x, mid_y] = renderer.center_position();
		renderer.draw_mesh(renderer.get_current_executor(), viking_mesh);

		static glm::vec3 pos {0,0,0};
		renderer.draw_planar_geometry(Geometry::Rectangle, { .position = pos, .dimensions = { { 1.f, 1.f, 1.f } } });
		pos += 0.001;
		// renderer.draw_text("Hello world!", 0, 0, 12.f);
		// static glm::vec3 pos_circle {-0.5,0,0};
		renderer.draw_planar_geometry(Geometry::Line, { .position = {-0.5, -0.5, 0}, .to_position = {0.5, 0.5, 0} });
		// renderer.draw_planar_geometry(Geometry::Circle, { .position = pos_circle, .dimensions = { { 1.f, 1.f, 1.f } } });
	}

	void destruct() override { Log::debug("Destructed AppLayer."); };

private:
	Ref<Device> device;
	Ref<PhysicalDevice> physical_device;
	Scope<Window>& window;
	Ref<Swapchain> swapchain;

	Ref<Pipeline> pipeline;

	Ref<Mesh> viking_mesh;
	Ref<Texture> viking_room;

};

int main(int argc, char** argv)
{
	struct A : public Panel {
		void construct(App&, Renderer& renderer) override { }
		void handle_swapchain_recreation(Renderer& renderer) override { }
		void update(float ts) override { }
		void update(float ts, Renderer& renderer) override { }
		void destruct() override { }
	};

	std::vector<std::string_view> args(argv, argv + argc);

	using namespace Disarray;
	App app;
	app.add_layer<AppLayer>();
	app.run();

}