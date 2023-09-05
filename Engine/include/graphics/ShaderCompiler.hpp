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
	void data_deleter(CompilerIntrinsics*);
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
	struct Deleter {
		void operator()(Detail::CompilerIntrinsics* ptr) { Detail::data_deleter(ptr); }
	};
	using CompilerData = std::unique_ptr<Detail::CompilerIntrinsics, Deleter>;
	CompilerData compiler_data { nullptr };
	void add_include_extension(std::vector<char>& glsl_code);
};

} // namespace Disarray::Runtime
