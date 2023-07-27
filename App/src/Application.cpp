#include <Disarray.hpp>
#include <vector>

class AppLayer : public Disarray::Layer {
public:
	AppLayer() = default;

	void construct() override { Disarray::Log::debug("Constructed AppLayer."); };
	void update(float ts) override {

	};

	/* Ideal API?
	void update(float ts, Disarray::Ref<Disarray::Renderer> renderer)
	{
		{
			// Pass is RAII for some object that draws the render pass upon destruction?
			const auto pass = renderer->begin_pass();
			const auto&& [mid_x, mid_y] = renderer->center_position();
			renderer->draw_mesh(my_mesh);
			renderer->draw_text("Hello world!", 0, 0, 12.f);
			renderer->draw_geometry(Geometry::Circle, { .pos = glm::vec3(mid_x, mid_y), .size = 12 });
		};
	}*/

	void destruct() override { Disarray::Log::debug("Destructed AppLayer."); };
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