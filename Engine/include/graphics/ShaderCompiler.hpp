#pragma once

#include <filesystem>
#include <memory>
#include <vector>

#include "Forward.hpp"

namespace Disarray {
enum class ShaderType : std::uint8_t;
}

namespace Disarray::Runtime {

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
	void add_include_extension(std::vector<char>& glsl_code);
	void add_include_extension(std::string& glsl_code);

	struct Deleter {
		void operator()(Detail::CompilerIntrinsics* ptr);
	};
	using CompilerData = Scope<Detail::CompilerIntrinsics, Deleter>;
	CompilerData compiler_data { nullptr };
};

} // namespace Disarray::Runtime
