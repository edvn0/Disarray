#include <Disarray.hpp>

class AppLayer : public Disarray::Layer {
public:
	AppLayer() = default;

	void construct() override { Disarray::Log::debug("Constructed AppLayer."); };
	void update(float ts) override {};
	void destruct() override { Disarray::Log::debug("Destructed AppLayer."); };
};

int main(int argc, char** argv)
{
	struct A { };
	using namespace Disarray;
	App app;
	app.add_layer<AppLayer>();
	app.run();
}