#pragma once

#include <filesystem>
#include <memory>
#include <vector>

#include "Forward.hpp"
#include "core/Collections.hpp"

namespace Disarray {
enum class ShaderType : std::uint8_t;
}

namespace Disarray::Runtime {

class BasicIncluder {
public:
	BasicIncluder(std::filesystem::path directory = "Assets/Shaders/Include");
	void replace_all_includes(std::string&);

private:
	auto check_and_replace(std::string& io_string, std::string_view to_find);

	Collections::StringMap<std::string> include_include_source_map {};
	static inline const Collections::StringViewSet extensions = { ".vert", ".frag", ".comp", ".glsl" };
};

namespace Detail {
	struct CompilerIntrinsics;
} // namespace Detail

class ShaderCompiler {
	using Code = std::vector<std::uint32_t>;

public:
	ShaderCompiler();
	auto compile(const std::filesystem::path&, ShaderType) -> Code;
	auto try_compile(const std::filesystem::path&, ShaderType) -> std::pair<bool, Code>;

	static void initialize();
	static void destroy();

private:
	void add_include_extension(std::string& glsl_code);

	BasicIncluder includer;

	struct Deleter {
		void operator()(Detail::CompilerIntrinsics* ptr);
	};
	using CompilerData = Scope<Detail::CompilerIntrinsics, Deleter>;
	CompilerData compiler_data { nullptr };
};

} // namespace Disarray::Runtime
