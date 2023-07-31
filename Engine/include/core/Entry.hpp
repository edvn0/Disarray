#ifdef DISARRAY_IMPLEMENTATION
#pragma once

#include <argparse/argparse.hpp>
#include <memory>

extern std::unique_ptr<Disarray::App> Disarray::create_application(const Disarray::ApplicationProperties&);

int main(int argc, char** argv)
{
	argparse::ArgumentParser program("Disarray", "1.0.0");

	program.add_argument("--width").help("Window width").default_value(1600u);
	program.add_argument("--height").help("Window height").default_value(900u);
	program.add_argument<std::string>("--name").help("Window name").default_value("Disarray");

	try {
		program.parse_args(argc, argv);
	} catch (const std::runtime_error& err) {
		std::cerr << err.what() << std::endl;
		std::cerr << program;
		return 1;
	}

	auto width = program.get<std::uint32_t>("width");
	auto height = program.get<std::uint32_t>("height");
	auto name = program.get<std::string>("name");

	const Disarray::ApplicationProperties properties {
		.width = width,
		.height = height,
		.name = std::string { name }
	};

	auto app = Disarray::create_application(properties);
	app->run();
}

#else
#error "Client include requires 'DISARRAY_IMPLEMENTATION' to be defined."
#endif