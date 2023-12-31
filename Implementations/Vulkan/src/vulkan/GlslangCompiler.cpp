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
		case ShaderType::Include:
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

static constexpr auto DEFAULT_RESOURCES = []() {
	return TBuiltInResource { .maxLights = 32,
		.maxClipPlanes = 6,
		.maxTextureUnits = 32,
		.maxTextureCoords = 32,
		.maxVertexAttribs = 64,
		.maxVertexUniformComponents = 4096,
		.maxVaryingFloats = 64,
		.maxVertexTextureImageUnits = 32,
		.maxCombinedTextureImageUnits = 80,
		.maxTextureImageUnits = 32,
		.maxFragmentUniformComponents = 4096,
		.maxDrawBuffers = 32,
		.maxVertexUniformVectors = 128,
		.maxVaryingVectors = 8,
		.maxFragmentUniformVectors = 16,
		.maxVertexOutputVectors = 16,
		.maxFragmentInputVectors = 15,
		.minProgramTexelOffset = -8,
		.maxProgramTexelOffset = 7,
		.maxClipDistances = 8,
		.maxComputeWorkGroupCountX = 65535,
		.maxComputeWorkGroupCountY = 65535,
		.maxComputeWorkGroupCountZ = 65535,
		.maxComputeWorkGroupSizeX = 1024,
		.maxComputeWorkGroupSizeY = 1024,
		.maxComputeWorkGroupSizeZ = 64,
		.maxComputeUniformComponents = 1024,
		.maxComputeTextureImageUnits = 16,
		.maxComputeImageUniforms = 8,
		.maxComputeAtomicCounters = 8,
		.maxComputeAtomicCounterBuffers = 1,
		.maxVaryingComponents = 60,
		.maxVertexOutputComponents = 64,
		.maxGeometryInputComponents = 64,
		.maxGeometryOutputComponents = 128,
		.maxFragmentInputComponents = 128,
		.maxImageUnits = 8,
		.maxCombinedImageUnitsAndFragmentOutputs = 8,
		.maxCombinedShaderOutputResources = 8,
		.maxImageSamples = 0,
		.maxVertexImageUniforms = 0,
		.maxTessControlImageUniforms = 0,
		.maxTessEvaluationImageUniforms = 0,
		.maxGeometryImageUniforms = 0,
		.maxFragmentImageUniforms = 8,
		.maxCombinedImageUniforms = 8,
		.maxGeometryTextureImageUnits = 16,
		.maxGeometryOutputVertices = 256,
		.maxGeometryTotalOutputComponents = 1024,
		.maxGeometryUniformComponents = 1024,
		.maxGeometryVaryingComponents = 64,
		.maxTessControlInputComponents = 128,
		.maxTessControlOutputComponents = 128,
		.maxTessControlTextureImageUnits = 16,
		.maxTessControlUniformComponents = 1024,
		.maxTessControlTotalOutputComponents = 4096,
		.maxTessEvaluationInputComponents = 128,
		.maxTessEvaluationOutputComponents = 128,
		.maxTessEvaluationTextureImageUnits = 16,
		.maxTessEvaluationUniformComponents = 1024,
		.maxTessPatchComponents = 120,
		.maxPatchVertices = 32,
		.maxTessGenLevel = 64,
		.maxViewports = 16,
		.maxVertexAtomicCounters = 0,
		.maxTessControlAtomicCounters = 0,
		.maxTessEvaluationAtomicCounters = 0,
		.maxGeometryAtomicCounters = 0,
		.maxFragmentAtomicCounters = 8,
		.maxCombinedAtomicCounters = 8,
		.maxAtomicCounterBindings = 1,
		.maxVertexAtomicCounterBuffers = 0,
		.maxTessControlAtomicCounterBuffers = 0,
		.maxTessEvaluationAtomicCounterBuffers = 0,
		.maxGeometryAtomicCounterBuffers = 0,
		.maxFragmentAtomicCounterBuffers = 1,
		.maxCombinedAtomicCounterBuffers = 1,
		.maxAtomicCounterBufferSize = 16384,
		.maxTransformFeedbackBuffers = 4,
		.maxTransformFeedbackInterleavedComponents = 64,
		.maxCullDistances = 8,
		.maxCombinedClipAndCullDistances = 8,
		.maxSamples = 4,
		.maxMeshOutputVerticesNV = 256,
		.maxMeshOutputPrimitivesNV = 512,
		.maxMeshWorkGroupSizeX_NV = 32,
		.maxMeshWorkGroupSizeY_NV = 1,
		.maxMeshWorkGroupSizeZ_NV = 1,
		.maxTaskWorkGroupSizeX_NV = 32,
		.maxTaskWorkGroupSizeY_NV = 1,
		.maxTaskWorkGroupSizeZ_NV = 1,
		.maxMeshViewCountNV = 4,
		.maxMeshOutputVerticesEXT = 256,
		.maxMeshOutputPrimitivesEXT = 256,
		.maxMeshWorkGroupSizeX_EXT = 128,
		.maxMeshWorkGroupSizeY_EXT = 128,
		.maxMeshWorkGroupSizeZ_EXT = 128,
		.maxTaskWorkGroupSizeX_EXT = 128,
		.maxTaskWorkGroupSizeY_EXT = 128,
		.maxTaskWorkGroupSizeZ_EXT = 128,
		.maxMeshViewCountEXT = 4,
		.maxDualSourceDrawBuffersEXT = 1,
		.limits = {
			.nonInductiveForLoops = true,
			.whileLoops = true,
			.doWhileLoops = true,
			.generalUniformIndexing = true,
			.generalAttributeMatrixVectorIndexing = true,
			.generalVaryingIndexing = true,
			.generalSamplerIndexing = true,
			.generalVariableIndexing = true,
			.generalConstantMatrixVectorIndexing = true,
		} };
};

auto ShaderCompiler::Deleter::operator()(Detail::CompilerIntrinsics* ptr) -> void { delete ptr; }

auto ShaderCompiler::compile(const std::filesystem::path& path_to_shader, ShaderType type) -> std::vector<std::uint32_t>
{
	Log::info("ShaderCompiler", "Compiling {}", path_to_shader);
	ensure(was_initialised(), "Compiler was not initialised");

	auto resources = DEFAULT_RESOURCES();
	auto& custom = resources;
	custom.maxCombinedTextureImageUnits = 2000;
	custom.maxTextureImageUnits = 2000;

	auto glslang_type = Detail::CompilerIntrinsics::to_glslang_type(type);
	Scope<glslang::TShader> shader = make_scope<glslang::TShader>(glslang_type);

	std::string output;
	if (!FS::read_from_file(path_to_shader.string(), output)) {
		throw CouldNotOpenStreamException { fmt::format("Could not read shader: {}", path_to_shader) };
	}
	add_include_extension(output);
	output.erase(std::remove(output.begin(), output.end(), '\0'), output.end());

	if (type == ShaderType::Include) {
		output += "void main() \n{}\n";
	}

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
	static constexpr std::string_view extension = "#version 460\n#extension GL_EXT_control_flow_attributes : require\n";
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
