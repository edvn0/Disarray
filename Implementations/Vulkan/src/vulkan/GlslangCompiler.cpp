#include "DisarrayPCH.hpp"

#include <SPIRV/GlslangToSpv.h>
#include <SPIRV/SpvTools.h>

#include <glslang/MachineIndependent/Versions.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>

#include <exception>
#include <filesystem>
#include <unordered_set>

#include "core/Collections.hpp"
#include "core/Ensure.hpp"
#include "core/Formatters.hpp"
#include "core/Log.hpp"
#include "core/Types.hpp"
#include "core/filesystem/FileIO.hpp"
#include "graphics/Shader.hpp"
#include "graphics/ShaderCompiler.hpp"
#include "vulkan/IncludeDirectoryIncluder.hpp"

namespace Disarray::Runtime {

static auto was_initialised() -> bool&
{
	static bool initialised { false };
	return std::ref(initialised);
}

struct Detail::CompilerIntrinsics {
	Scope<IncludeDirectoryIncluder> include_dir_includer = make_scope<IncludeDirectoryIncluder>("Assets/Shaders/Include");

	static auto to_glslang_type(ShaderType type) -> EShLanguage
	{
		switch (type) {
		case ShaderType::Vertex:
			return EShLangVertex;
		case ShaderType::Fragment:
			return EShLangFragment;
		case ShaderType::Compute:
			return EShLangCompute;
		default:
			unreachable("Incorrect mapping of shader types.");
		}
	}
};

auto ShaderCompiler::Deleter::operator()(Detail::CompilerIntrinsics* ptr) -> void { delete ptr; }

auto ShaderCompiler::compile(const std::filesystem::path& path_to_shader, ShaderType type) -> std::vector<std::uint32_t>
{
	Log::info("ShaderCompiler", "Compiling {}", path_to_shader);
	ensure(was_initialised(), "Compiler was not initialised");

	const TBuiltInResource* resources = GetDefaultResources();
	auto custom = *resources;
	custom.maxCombinedTextureImageUnits = 2000;
	custom.maxTextureImageUnits = 2000;

	auto glslang_type = Detail::CompilerIntrinsics::to_glslang_type(type);
	Scope<glslang::TShader> shader = make_scope<glslang::TShader>(glslang_type);

	std::string output;
	if (!FS::read_from_file(path_to_shader.string(), output)) {
		throw CouldNotOpenStreamException { fmt::format("Could not read shader: {}", path_to_shader) };
	}
	add_include_extension(output);

	std::array sources = { output.c_str() };
	shader->setStrings(sources.data(), 1);

	// Use appropriate Vulkan version
	glslang::EShTargetClientVersion target_api_version = glslang::EShTargetVulkan_1_3;
	shader->setEnvClient(glslang::EShClientVulkan, target_api_version);

	glslang::EShTargetLanguageVersion target_spirv_version = glslang::EShTargetSpv_1_6;
	shader->setEnvTarget(glslang::EshTargetSpv, target_spirv_version);

	shader->setEntryPoint("main");
	const int default_version = 460;
	const bool forward_compatible = false;
	EProfile default_profile = ECoreProfile;

	std::string preprocessed_str;
	if (glslang::TShader::ForbidIncluder forbid_includer {}; !shader->preprocess(
			&custom, default_version, default_profile, false, forward_compatible, EShMsgDefault, &preprocessed_str, forbid_includer)) {
		Log::error("ShaderCompiler", "Could not preprocess shader: {}, because {}", path_to_shader.string(), shader->getInfoLog());
		return {};
	}

	std::array strings { preprocessed_str.c_str() };
	shader->setStrings(strings.data(), 1);

	if (!shader->parse(&custom, default_version, default_profile, false, forward_compatible, EShMsgDefault)) {
		Log::error("ShaderCompiler", "Could not parse shader: {}, because: {}", path_to_shader.string(), shader->getInfoLog());
		return {};
	}

	glslang::TProgram program;
	program.addShader(shader.get());
	if (!program.link(EShMsgDefault)) {
		Log::error("ShaderCompiler", "Could not link shader: {}, because {}", path_to_shader.string(), program.getInfoLog());
		return {};
	}

	const auto& intermediate_ref = *(program.getIntermediate(shader->getStage()));
	std::vector<uint32_t> spirv;
	glslang::SpvOptions options {};
	options.validate = true;
	options.generateDebugInfo = true;
	spv::SpvBuildLogger logger;
	glslang::GlslangToSpv(intermediate_ref, spirv, &logger, &options);

	return spirv;
}

auto ShaderCompiler::try_compile(const std::filesystem::path& path, Disarray::ShaderType type) -> std::pair<bool, Code>
{
	try {
		auto compiled = compile(path, type);
		if (compiled.empty()) {
			return { false, {} };
		}
		return { true, compiled };

	} catch (const BaseException&) {
		return { false, {} };
	} catch (const std::exception&) {
		return { false, {} };
	}
}

void ShaderCompiler::initialize()
{
	if (!was_initialised()) {
		glslang::InitializeProcess();
		was_initialised() = true;
	}
}
void ShaderCompiler::destroy()
{
	if (was_initialised()) {
		glslang::FinalizeProcess();
		was_initialised() = false;
	}
}

static auto replace(std::string& output, const std::string_view from, const std::string& replacement) -> bool
{
	size_t start_pos = output.find(from);
	if (start_pos == std::string::npos) {
		return false;
	}
	output.replace(start_pos, from.length(), replacement.c_str());
	return true;
}

auto BasicIncluder::check_and_replace(std::string& io_string, std::string_view to_find)
{
	const auto include = fmt::format("#include \"{}\"\n", to_find);
	if (io_string.find(include) == std::string::npos) {
		return;
	}

	ensure(include_include_source_map.contains(to_find), fmt::format("Could not find key {}", to_find));
	const auto& include_string = include_include_source_map.at(std::string { to_find });

	replace(io_string, include, fmt::format("\n{}\n", include_string));
}

BasicIncluder::BasicIncluder(std::filesystem::path directory)
{
	FS::for_each_in_directory(
		std::move(directory),
		[&inc = include_include_source_map](const std::filesystem::directory_entry& entry) {
			const auto& path = entry.path();
			inc[path.filename().string()] = {};
			if (!FS::read_from_file(path.string(), inc[path.filename().string()])) {
				Log::error("BasicIncluder", "Could not read file {}", path.string());
			}
		},
		[](const std::filesystem::directory_entry& entry) { return extensions.contains(entry.path().extension().string()); });

	ensure(include_include_source_map.size() == 8, "We currently have 8 include files");
}

void BasicIncluder::replace_all_includes(std::string& io_string)
{
	Collections::for_each(include_include_source_map, [&str = io_string, this](const auto& pair) {
		auto&& [view, string] = pair;
		check_and_replace(str, view);
	});
}

void ShaderCompiler::add_include_extension(std::string& glsl_code)
{
	ensure(glsl_code.find("#version") == std::string::npos, "Shader already has a #version directive");
	static constexpr std::string_view extension = "#version 460\n";
	glsl_code.insert(0, extension);

	using namespace std::string_view_literals;
	includer.replace_all_includes(glsl_code);
}

ShaderCompiler::ShaderCompiler()
	: includer("Assets/Shaders/Include")
	, compiler_data(make_scope<Detail::CompilerIntrinsics, Deleter>())
{
}

} // namespace Disarray::Runtime
