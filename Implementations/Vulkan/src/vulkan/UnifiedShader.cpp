#include "DisarrayPCH.hpp"

#include <spirv_reflect.hpp>
#include <vulkan/vulkan_core.h>

#include <vector>

#include "SPIRV/GlslangToSpv.h"
#include "SPIRV/SpvTools.h"
#include "core/filesystem/AssetLocations.hpp"
#include "core/filesystem/FileIO.hpp"
#include "graphics/ShaderCompiler.hpp"
#include "vulkan/UnifiedShader.hpp"

namespace Disarray::Vulkan {

namespace {
	auto to_glslang_type(ShaderType type) -> EShLanguage
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

	constexpr auto default_resources()
	{
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

} // namespace

auto split_string(const std::string& input, const std::string& delimiter) -> std::vector<std::string>
{
	std::vector<std::string> split_strings;
	std::size_t pos = 0;
	std::size_t found;

	while ((found = input.find(delimiter, pos)) != std::string::npos) {
		if (auto token = input.substr(pos, found - pos); !token.empty()) {
			split_strings.push_back(token);
		}
		pos = found + delimiter.length();
	}

	// Add the last token
	if (const auto last_token = input.substr(pos); !last_token.empty()) {
		split_strings.push_back(last_token);
	}

	return split_strings;
}

UnifiedIncluder::UnifiedIncluder(const std::filesystem::path& dir)
	: directory(dir)
{
}

UnifiedIncluder::~UnifiedIncluder() = default;

UnifiedIncluder::IncludeResult* UnifiedIncluder::includeSystem(const char* header_name, const char* /*includerName*/, size_t /*inclusionDepth*/)
{
	const auto& source = get_or_emplace_include(header_name);
	return new IncludeResult { header_name, source.c_str(), source.size(), nullptr };
}

auto UnifiedIncluder::get_or_emplace_include(const std::string& header_name) -> const std::string&
{
	if (sources.contains(header_name)) {
		return sources.at(header_name);
	}

	const auto fixed = FS::shader_directory() / std::filesystem::path { "Include" } / header_name;
	if (!FS::exists(fixed)) {
		Log::error("UnifiedIncluder", "Could not find include: {}", header_name);
		throw CouldNotOpenStreamException { "Could not find include: " + header_name };
	}

	std::string read;
	if (!FS::read_from_file(fixed.string(), read)) {
		Log::error("UnifiedIncluder", "Could not read include: {}", header_name);
		throw CouldNotOpenStreamException { "Could not find include: " + header_name };
	}
	read += '\n';
	read += '\0';
	read = read.c_str();
	sources.emplace(header_name, read);

	const auto& found = sources.at(header_name);
	return found;
}

UnifiedIncluder::IncludeResult* UnifiedIncluder::includeLocal(const char* header_name, const char* includer_name, std::size_t depth)
{
	const auto& source = get_or_emplace_include(header_name);
	return new IncludeResult { header_name, source.c_str(), source.size(), nullptr };
}

void UnifiedIncluder::releaseInclude(IncludeResult* result) { delete result; }

UnifiedShader::UnifiedShader(const Disarray::Device& dev, UnifiedShaderProperties properties)
	: Disarray::UnifiedShader(properties)
	, device(dev)
{
	std::string read_file;
	if (!FS::read_from_file(properties.path.string(), read_file)) {
		Log::error("UnifiedShader", "Failed to read shader file: {}", properties.path.string());
		return;
	}
	using namespace std::string_view_literals;
	static constexpr auto vertex_delimiter = "#pragma stage vertex"sv;
	static constexpr auto fragment_delimiter = "#pragma stage fragment"sv;
	static constexpr auto vertex_delimiter_size = vertex_delimiter.size();

	auto split = split_string(read_file, std::string { fragment_delimiter });

	auto vertex = split.at(0);
	// Add '#extension GL_ARB_shading_language_include : require' at offset 0 to vertex
	vertex.erase(0, vertex_delimiter_size);
	vertex.insert(0, "#extension GL_GOOGLE_include_directive : require\n#extension GL_EXT_control_flow_attributes : require\n");
	auto fragment = split.at(1);
	fragment.insert(0, "#extension GL_GOOGLE_include_directive : require\n#extension GL_EXT_control_flow_attributes : require\n");

	sources[ShaderType::Vertex] = vertex;
	sources[ShaderType::Fragment] = fragment;

	// We now need a includer for the shader compiler
	UnifiedIncluder includer { "Assets/Shaders/Include" };

	glslang::TProgram program;
	std::unordered_map<ShaderType, Scope<glslang::TShader>> shaders {};
	for (const auto& kv : sources) {
		auto&& [type, source] = kv;
		auto resources = default_resources();
		auto& custom = resources;
		custom.maxCombinedTextureImageUnits = 2000;
		custom.maxTextureImageUnits = 2000;

		auto glslang_type = to_glslang_type(type);
		auto shader = make_scope<glslang::TShader>(glslang_type);

		const std::array shader_sources = { source.c_str() };
		shader->setStrings(shader_sources.data(), 1);

		// Use appropriate Vulkan version
		constexpr glslang::EShTargetClientVersion target_api_version = glslang::EShTargetVulkan_1_3;
		constexpr glslang::EShTargetLanguageVersion target_spirv_version = glslang::EShTargetSpv_1_6;
		shader->setEnvClient(glslang::EShClientVulkan, target_api_version);
		shader->setEnvTarget(glslang::EshTargetSpv, target_spirv_version);

		shader->setEntryPoint("main");
		constexpr int default_version = 460;
		constexpr bool forward_compatible = false;
		constexpr EProfile default_profile = ECoreProfile;

		std::string preprocessed_str;
		if (!shader->preprocess(&custom, default_version, default_profile, false, forward_compatible, EShMsgDefault, &preprocessed_str, includer)) {
			Log::error("ShaderCompiler", "Could not preprocess shader: {}, because {}", props.path.string(), shader->getInfoLog());
			throw;
		}

		std::array strings { preprocessed_str.c_str() };
		shader->setStrings(strings.data(), 1);

		if (!shader->parse(&custom, default_version, default_profile, false, forward_compatible, EShMsgDefault)) {
			Log::error("ShaderCompiler", "Could not parse shader: {}, because: {}", props.path.string(), shader->getInfoLog());
			throw;
		}

		shaders.emplace(type, std::move(shader));
	}

	for (const auto& kv : shaders) {
		auto&& [type, shader] = kv;
		program.addShader(shader.get());
	}

	if (!program.link(EShMsgDefault)) {
		Log::error("ShaderCompiler", "Could not link shader: {}, because {}", props.path.string(), program.getInfoLog());
		throw;
	}

	glslang::SpvOptions spv_options;
	spv_options.generateDebugInfo = !props.optimize;
	spv_options.disableOptimizer = !props.optimize;
	spv_options.optimizeSize = !props.optimize;
	spv_options.disassemble = false;
	// Emplace an item to spirv_sources with key ShaderType::Vertex
	spirv_sources[ShaderType::Vertex] = {};
	GlslangToSpv(*program.getIntermediate(to_glslang_type(ShaderType::Vertex)), spirv_sources.at(ShaderType::Vertex), &spv_options);
	spirv_sources[ShaderType::Fragment] = {};
	GlslangToSpv(*program.getIntermediate(to_glslang_type(ShaderType::Fragment)), spirv_sources.at(ShaderType::Fragment), &spv_options);
}

UnifiedShader::~UnifiedShader() { }

} // namespace Disarray::Vulkan
