// clang-format off
#include "core/App.hpp"
#define DISARRAY_IMPLEMENTATION
#include "core/Entry.hpp"

#include "ClientLayer.hpp"
// clang-format on

class ClientApp : public Disarray::App {
public:
	using Disarray::App::App;

	void on_attach() override { add_layer<Disarray::Client::ClientLayer>(); }
	void on_detach() override { }
};

extern auto Disarray::create_application(const Disarray::ApplicationProperties& props) -> Disarray::Scope<Disarray::App, Disarray::AppDeleter>
{
	return Disarray::make_scope<ClientApp, Disarray::AppDeleter>(props);
}
