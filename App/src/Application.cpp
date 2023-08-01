// clang-format off
#include "core/App.hpp"
#define DISARRAY_IMPLEMENTATION
#include "core/Entry.hpp"

#include "ClientLayer.hpp"
// clang-format on

class ClientApp : public Disarray::App {
public:
	using Disarray::App::App;

	void on_attach() override { add_layer<Disarray::Client::AppLayer>(); }
};

extern std::unique_ptr<Disarray::App> Disarray::create_application(const Disarray::ApplicationProperties& props)
{
	return std::make_unique<ClientApp>(props);
}
