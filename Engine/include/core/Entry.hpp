#ifdef DISARRAY_IMPLEMENTATION
#pragma once

#include <argparse/argparse.hpp>

#include <filesystem>
#include <memory>
#include <string>

extern auto Disarray::create_application(const Disarray::ApplicationProperties&) -> Disarray::Scope<Disarray::App, Disarray::AppDeleter>;

int main(int argc, char** argv)
{
	argparse::ArgumentParser program("Disarray", "1.0.0");

	const auto current_path = std::filesystem::current_path();

	program.add_argument("--width").help("Window width").default_value(1600u);
	program.add_argument("--height").help("Window height").default_value(900u);
	program.add_argument<std::string>("--wd").help("Working directory").default_value(current_path.string());
	program.add_argument<std::string>("--name").help("Window name").default_value(std::string { "Disarray" });
	program.add_argument("--fullscreen").help("Start in fullscreen").default_value(false).implicit_value(true);

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
	auto working_directory = program.get<std::string>("wd");
	auto is_fullscreen = program["--fullscreen"] == true;
	const Disarray::ApplicationProperties properties {
		.width = width,
		.height = height,
		.name = std::string { name },
		.is_fullscreen = is_fullscreen,
		.working_directory = std::filesystem::path { working_directory },
	};

	auto app = Disarray::create_application(properties);
	app->run();
}

#else
#error "Client include requires 'DISARRAY_IMPLEMENTATION' to be defined."
#endif
