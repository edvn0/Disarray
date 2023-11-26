#include "DisarrayPCH.hpp"

#include <spirv_reflect.hpp>
#include <vulkan/vulkan_core.h>

#include <glslang/Public/ShaderLang.h>

#include <vector>

#include "SPIRV/GlslangToSpv.h"
#include "SPIRV/SpvTools.h"
#include "core/Ensure.hpp"
#include "core/filesystem/AssetLocations.hpp"
#include "core/filesystem/FileIO.hpp"
#include "graphics/ShaderCompiler.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Shader.hpp"
#include "vulkan/UnifiedShader.hpp"

namespace Disarray::Vulkan {

class UnifiedIncluder final : public glslang::TShader::Includer {
public:
	explicit UnifiedIncluder(const std::filesystem::path& dir);
	~UnifiedIncluder() override;

	auto includeSystem(const char*, const char*, std::size_t) -> IncludeResult* override;
	auto includeLocal(const char*, const char*, std::size_t) -> IncludeResult* override;
	void releaseInclude(IncludeResult*) override;

private:
	auto get_or_emplace_include(const std::string& header_name) -> const std::string&;

	Collections::StringMap<std::string> sources {};
	std::filesystem::path directory;
};

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
	}

	auto to_vulkan_shader_type(const ShaderType type) -> VkShaderStageFlagBits
	{
		switch (type) {
		case ShaderType::Vertex:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case ShaderType::Fragment:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		case ShaderType::Compute:
			return VK_SHADER_STAGE_COMPUTE_BIT;
		default:
			unreachable();
		}
	}

	std::unordered_map<std::uint32_t, std::unordered_map<std::uint32_t, Reflection::UniformBuffer>> global_uniform_buffers {};
	std::unordered_map<std::uint32_t, std::unordered_map<std::uint32_t, Reflection::StorageBuffer>> global_storage_buffers {};

	auto spir_type_to_shader_uniform_type(const spirv_cross::SPIRType& type) -> Reflection::ShaderUniformType
	{
		switch (type.basetype) {
		case spirv_cross::SPIRType::Boolean:
			return Reflection::ShaderUniformType::Bool;
		case spirv_cross::SPIRType::Int:
			if (type.vecsize == 1) {
				return Reflection::ShaderUniformType::Int;
			}
			if (type.vecsize == 2) {
				return Reflection::ShaderUniformType::IVec2;
			}
			if (type.vecsize == 3) {
				return Reflection::ShaderUniformType::IVec3;
			}
			if (type.vecsize == 4) {
				return Reflection::ShaderUniformType::IVec4;
			}

		case spirv_cross::SPIRType::UInt:
			return Reflection::ShaderUniformType::UInt;
		case spirv_cross::SPIRType::Float:
			if (type.columns == 3) {
				return Reflection::ShaderUniformType::Mat3;
			}
			if (type.columns == 4) {
				return Reflection::ShaderUniformType::Mat4;
			}

			if (type.vecsize == 1) {
				return Reflection::ShaderUniformType::Float;
			}
			if (type.vecsize == 2) {
				return Reflection::ShaderUniformType::Vec2;
			}
			if (type.vecsize == 3) {
				return Reflection::ShaderUniformType::Vec3;
			}
			if (type.vecsize == 4) {
				return Reflection::ShaderUniformType::Vec4;
			}
			break;
		default: {
			ensure(false, "Unknown type!");
			return Reflection::ShaderUniformType::None;
		}
		}
		ensure(false, "Unknown type!");
		return Reflection::ShaderUniformType::None;
	}

	auto to_stage(ShaderType type)
	{
		switch (type) {
		case ShaderType::Vertex:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case ShaderType::Fragment:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		case ShaderType::Compute:
			return VK_SHADER_STAGE_COMPUTE_BIT;
		case ShaderType::Include:
			return VK_SHADER_STAGE_ALL;
		default:
			unreachable("Could not map to Vulkan stage");
		}
	}

	auto reflect_code(VkShaderStageFlagBits shaderStage, const std::vector<std::uint32_t>& spirv, ReflectionData& output) -> void
	{
		const spirv_cross::Compiler compiler(spirv);
		auto resources = compiler.get_shader_resources();

		for (const auto& resource : resources.uniform_buffers) {
			auto active_buffers = compiler.get_active_buffer_ranges(resource.id);
			if (active_buffers.empty()) {
				continue;
			}
			// Discard unused buffers from headers
			const auto& name = resource.name;
			const auto& buffer_type = compiler.get_type(resource.base_type_id);
			// auto member_count = static_cast<std::uint32_t>(buffer_type.member_types.size());
			auto binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			auto descriptor_set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			auto size = static_cast<std::uint32_t>(compiler.get_declared_struct_size(buffer_type));

			if (descriptor_set >= output.ShaderDescriptorSets.size()) {
				output.ShaderDescriptorSets.resize(descriptor_set + 1);
			}

			Reflection::ShaderDescriptorSet& shader_descriptor_set = output.ShaderDescriptorSets[descriptor_set];
			if (!global_uniform_buffers[descriptor_set].contains(binding)) {
				Reflection::UniformBuffer uniform_buffer;
				uniform_buffer.BindingPoint = binding;
				uniform_buffer.Size = size;
				uniform_buffer.Name = name;
				uniform_buffer.ShaderStage = VK_SHADER_STAGE_ALL;
				global_uniform_buffers.at(descriptor_set)[binding] = uniform_buffer;
			} else {
				Reflection::UniformBuffer& uniform_buffer = global_uniform_buffers.at(descriptor_set).at(binding);
				if (size > uniform_buffer.Size) {
					uniform_buffer.Size = size;
				}
			}
			shader_descriptor_set.UniformBuffers[binding] = global_uniform_buffers.at(descriptor_set).at(binding);
		}

		for (const auto& resource : resources.storage_buffers) {
			auto active_buffers = compiler.get_active_buffer_ranges(resource.id);
			if (active_buffers.empty()) {
				continue;
			}

			const auto& name = resource.name;
			const auto& buffer_type = compiler.get_type(resource.base_type_id);
			// auto member_count = static_cast<std::uint32_t>(buffer_type.member_types.size());
			auto binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			auto descriptor_set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			auto size = static_cast<std::uint32_t>(compiler.get_declared_struct_size(buffer_type));

			if (descriptor_set >= output.ShaderDescriptorSets.size()) {
				output.ShaderDescriptorSets.resize(descriptor_set + 1);
			}

			Reflection::ShaderDescriptorSet& shader_descriptor_set = output.ShaderDescriptorSets[descriptor_set];
			if (!global_storage_buffers[descriptor_set].contains(binding)) {
				Reflection::StorageBuffer storage_buffer;
				storage_buffer.BindingPoint = binding;
				storage_buffer.Size = size;
				storage_buffer.Name = name;
				storage_buffer.ShaderStage = VK_SHADER_STAGE_ALL;
				global_storage_buffers.at(descriptor_set)[binding] = storage_buffer;
			} else {
				Reflection::StorageBuffer& storage_buffer = global_storage_buffers.at(descriptor_set).at(binding);
				if (size > storage_buffer.Size) {
					storage_buffer.Size = size;
				}
			}

			shader_descriptor_set.StorageBuffers[binding] = global_storage_buffers.at(descriptor_set).at(binding);
		}

		for (const auto& resource : resources.push_constant_buffers) {
			const auto& buffer_name = resource.name;
			const auto& buffer_type = compiler.get_type(resource.base_type_id);
			auto buffer_size = static_cast<std::uint32_t>(compiler.get_declared_struct_size(buffer_type));
			auto member_count = static_cast<std::uint32_t>(buffer_type.member_types.size());
			auto buffer_offset = 0U;
			if (!output.PushConstantRanges.empty()) {
				buffer_offset = output.PushConstantRanges.back().Offset + output.PushConstantRanges.back().Size;
			}

			auto& push_constant_range = output.PushConstantRanges.emplace_back();
			push_constant_range.ShaderStage = shaderStage;
			push_constant_range.Size = buffer_size - buffer_offset;
			push_constant_range.Offset = buffer_offset;

			// Skip empty push constant buffers - these are for the renderer only
			if (buffer_name.empty()) {
				continue;
			}

			Reflection::ShaderBuffer& buffer = output.constant_buffers[buffer_name];
			buffer.Name = buffer_name;
			buffer.Size = buffer_size - buffer_offset;

			for (auto i = 0U; i < member_count; i++) {
				const auto& type = compiler.get_type(buffer_type.member_types[i]);
				const auto& member_name = compiler.get_member_name(buffer_type.self, i);
				auto size = static_cast<std::uint32_t>(compiler.get_declared_struct_member_size(buffer_type, i));
				auto offset = compiler.type_struct_member_offset(buffer_type, i) - buffer_offset;

				const auto uniform_name = fmt::format("{}.{}", buffer_name, member_name);
				const auto spirv_type = spir_type_to_shader_uniform_type(type);
				buffer.Uniforms[uniform_name] = Reflection::ShaderUniform(uniform_name, spirv_type, size, offset);
			}
		}

		for (const auto& resource : resources.sampled_images) {
			const auto& name = resource.name;
			// const auto& base_type = compiler.get_type(resource.base_type_id);
			const auto& type = compiler.get_type(resource.type_id);
			auto binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			auto descriptor_set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			// auto dimension = base_type.image.dim;
			auto array_size = type.array[0];
			if (array_size == 0) {
				array_size = 1;
			}
			if (descriptor_set >= output.ShaderDescriptorSets.size()) {
				output.ShaderDescriptorSets.resize(descriptor_set + 1);
			}

			Reflection::ShaderDescriptorSet& shader_descriptor_set = output.ShaderDescriptorSets[descriptor_set];
			auto& image_sampler = shader_descriptor_set.ImageSamplers[binding];
			image_sampler.BindingPoint = binding;
			image_sampler.DescriptorSet = descriptor_set;
			image_sampler.Name = name;
			image_sampler.ShaderStage = shaderStage;
			image_sampler.ArraySize = array_size;

			output.resources[name] = Reflection::ShaderResourceDeclaration(name, binding, 1);
		}

		for (const auto& resource : resources.separate_images) {
			const auto& name = resource.name;
			// const auto& base_type = compiler.get_type(resource.base_type_id);
			const auto& type = compiler.get_type(resource.type_id);
			auto binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			auto descriptor_set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			// auto dimension = base_type.image.dim;
			auto array_size = type.array[0];
			if (array_size == 0) {
				array_size = 1;
			}
			if (descriptor_set >= output.ShaderDescriptorSets.size()) {
				output.ShaderDescriptorSets.resize(descriptor_set + 1);
			}

			Reflection::ShaderDescriptorSet& shader_descriptor_set = output.ShaderDescriptorSets[descriptor_set];
			auto& image_sampler = shader_descriptor_set.SeparateTextures[binding];
			image_sampler.BindingPoint = binding;
			image_sampler.DescriptorSet = descriptor_set;
			image_sampler.Name = name;
			image_sampler.ShaderStage = shaderStage;
			image_sampler.ArraySize = array_size;

			output.resources[name] = Reflection::ShaderResourceDeclaration(name, binding, 1);
		}

		for (const auto& resource : resources.separate_samplers) {
			const auto& name = resource.name;
			// const auto& base_type = compiler.get_type(resource.base_type_id);
			const auto& type = compiler.get_type(resource.type_id);
			auto binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			auto descriptor_set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			auto array_size = type.array[0];
			if (array_size == 0) {
				array_size = 1;
			}
			if (descriptor_set >= output.ShaderDescriptorSets.size()) {
				output.ShaderDescriptorSets.resize(descriptor_set + 1);
			}

			Reflection::ShaderDescriptorSet& shader_descriptor_set = output.ShaderDescriptorSets[descriptor_set];
			auto& image_sampler = shader_descriptor_set.SeparateSamplers[binding];
			image_sampler.BindingPoint = binding;
			image_sampler.DescriptorSet = descriptor_set;
			image_sampler.Name = name;
			image_sampler.ShaderStage = shaderStage;
			image_sampler.ArraySize = array_size;

			output.resources[name] = Reflection::ShaderResourceDeclaration(name, binding, 1);
		}

		for (const auto& resource : resources.storage_images) {
			const auto& name = resource.name;
			const auto& type = compiler.get_type(resource.type_id);
			auto binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			auto descriptor_set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			// auto dimension = type.image.dim;
			auto array_size = type.array[0];
			if (array_size == 0) {
				array_size = 1;
			}
			if (descriptor_set >= output.ShaderDescriptorSets.size()) {
				output.ShaderDescriptorSets.resize(descriptor_set + 1);
			}

			Reflection::ShaderDescriptorSet& shader_descriptor_set = output.ShaderDescriptorSets[descriptor_set];
			auto& image_sampler = shader_descriptor_set.StorageImages[binding];
			image_sampler.BindingPoint = binding;
			image_sampler.DescriptorSet = descriptor_set;
			image_sampler.Name = name;
			image_sampler.ArraySize = array_size;
			image_sampler.ShaderStage = shaderStage;

			output.resources[name] = Reflection::ShaderResourceDeclaration(name, binding, 1);
		}
	}

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

auto UnifiedIncluder::includeSystem(const char* header_name, const char* /*includerName*/, size_t /*inclusionDepth*/)
	-> UnifiedIncluder::IncludeResult*
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
	read = std::string { read.c_str() };
	sources.emplace(header_name, read);

	const auto& found = sources.at(header_name);
	return found;
}

auto UnifiedIncluder::includeLocal(const char* header_name, const char* includer_name, std::size_t depth) -> UnifiedIncluder::IncludeResult*
{
	const auto& source = get_or_emplace_include(header_name);
	return new IncludeResult { header_name, source.c_str(), source.size(), nullptr };
}

void UnifiedIncluder::releaseInclude(IncludeResult* result) { delete result; }

UnifiedShader::UnifiedShader(const Disarray::Device& dev, UnifiedShaderProperties in_props)
	: Disarray::UnifiedShader(std::move(in_props))
	, device(dev)
{
	recreate(false, {});
}

auto UnifiedShader::clean_shader() -> void
{
	sources.clear();
	spirv_sources.clear();
	for (const auto& kv : shader_modules) {
		auto&& [type, module] = kv;
		vkDestroyShaderModule(supply_cast<Vulkan::Device>(device), module, nullptr);
	}
	for (const auto& layout : descriptor_set_layouts) {
		vkDestroyDescriptorSetLayout(supply_cast<Vulkan::Device>(device), layout, nullptr);
	}
	shader_modules.clear();
	stage_infos.clear();
}

UnifiedShader::~UnifiedShader() { clean_shader(); };

void UnifiedShader::recreate(bool should_clean, const Extent&)
{
	if (should_clean) {
		clean_shader();
	}
	std::string read_file;
	if (!FS::read_from_file(props.path.string(), read_file)) {
		Log::error("UnifiedShader", "Failed to read shader file: {}", props.path.string());
		return;
	}
	using namespace std::string_view_literals;
	static constexpr auto vertex_delimiter = "#pragma stage vertex"sv;
	static constexpr auto fragment_delimiter = "#pragma stage fragment"sv;
	static constexpr auto vertex_delimiter_size = vertex_delimiter.size();

	auto split = split_string(read_file, std::string { fragment_delimiter });

	auto vertex = split.at(0);
	vertex.erase(0, vertex_delimiter_size);
	auto fragment = split.at(1);

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

		shader->setPreamble("#extension GL_GOOGLE_include_directive : require\n#extension GL_EXT_control_flow_attributes : require\n");

		std::string preprocessed_str;
		if (!shader->preprocess(&custom, default_version, default_profile, false, forward_compatible, EShMsgDefault, &preprocessed_str, includer)) {
			Log::error("ShaderCompiler", "Could not preprocess shader: {}, because {}", props.path.string(), shader->getInfoLog());
			throw CouldNotCompileShaderException { fmt::format(
				"Could not preprocess shader: {}, because {}", props.path.string(), shader->getInfoLog()) };
		}

		std::array strings { preprocessed_str.c_str() };
		shader->setStrings(strings.data(), 1);

		if (!shader->parse(&custom, default_version, default_profile, false, forward_compatible, EShMsgDefault)) {
			Log::error("ShaderCompiler", "Could not parse shader: {}, because: {}", props.path.string(), shader->getInfoLog());
			throw CouldNotCompileShaderException { fmt::format(
				"Could not parse shader: {}, because: {}", props.path.string(), shader->getInfoLog()) };
		}

		shaders.emplace(type, std::move(shader));
	}

	for (const auto& kv : shaders) {
		auto&& [type, shader] = kv;
		program.addShader(shader.get());
	}

	if (!program.link(EShMsgDefault)) {
		Log::error("ShaderCompiler", "Could not link shader: {}, because {}", props.path.string(), program.getInfoLog());
		throw CouldNotCompileShaderException { fmt::format("Could not link shader: {}, because {}", props.path.string(), program.getInfoLog()) };
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

	create_vulkan_objects();
}

auto reflect_on_stages(const std::unordered_map<Disarray::ShaderType, std::vector<std::uint32_t>>& spirv_codes) -> ReflectionData
{
	ReflectionData output {};
	for (const auto& kv : spirv_codes) {
		auto&& [type, stage_info] = kv;
		reflect_code(to_stage(type), stage_info, output);
	}
	return output;
}

auto UnifiedShader::create_vulkan_objects() -> void
{
	for (const auto& kv : spirv_sources) {
		auto&& [type, spirv] = kv;
		VkShaderModuleCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.codeSize = spirv.size() * sizeof(std::uint32_t);
		create_info.pCode = spirv.data();

		VkShaderModule shader_module;
		if (vkCreateShaderModule(supply_cast<Vulkan::Device>(device), &create_info, nullptr, &shader_module) != VK_SUCCESS) {
			Log::error("UnifiedShader", "Failed to create shader module!");
			throw CouldNotCompileShaderException { "Failed to create shader module!" };
		}

		shader_modules.emplace(type, std::move(shader_module));
		stage_infos.emplace(type,
			VkPipelineShaderStageCreateInfo {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage = to_vulkan_shader_type(type),
				.module = shader_modules.at(type),
				.pName = "main",
			});
	}

	reflection_data = reflect_on_stages(spirv_sources);
	create_descriptor_set_layouts();
}

auto UnifiedShader::create_descriptor_set_layouts() -> void
{
	auto* vk_device = supply_cast<Vulkan::Device>(device);

	for (std::uint32_t set = 0; set < reflection_data.ShaderDescriptorSets.size(); set++) {
		auto& shader_descriptor_set = reflection_data.ShaderDescriptorSets[set];

		std::vector<VkDescriptorSetLayoutBinding> layout_bindings {};
		for (auto& [binding, uniformBuffer] : shader_descriptor_set.UniformBuffers) {
			VkDescriptorSetLayoutBinding& layoutBinding = layout_bindings.emplace_back();
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			layoutBinding.descriptorCount = 1;
			layoutBinding.stageFlags = uniformBuffer.ShaderStage;
			layoutBinding.pImmutableSamplers = nullptr;
			layoutBinding.binding = binding;

			VkWriteDescriptorSet& write_set = shader_descriptor_set.WriteDescriptorSets[uniformBuffer.Name];
			write_set = {};
			write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_set.descriptorType = layoutBinding.descriptorType;
			write_set.descriptorCount = 1;
			write_set.dstBinding = layoutBinding.binding;
		}

		for (auto& [binding, storageBuffer] : shader_descriptor_set.StorageBuffers) {
			VkDescriptorSetLayoutBinding& layoutBinding = layout_bindings.emplace_back();
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			layoutBinding.descriptorCount = 1;
			layoutBinding.stageFlags = storageBuffer.ShaderStage;
			layoutBinding.pImmutableSamplers = nullptr;
			layoutBinding.binding = binding;
			ensure(!shader_descriptor_set.UniformBuffers.contains(binding), "Binding is already present!");

			VkWriteDescriptorSet& write_set = shader_descriptor_set.WriteDescriptorSets[storageBuffer.Name];
			write_set = {};
			write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_set.descriptorType = layoutBinding.descriptorType;
			write_set.descriptorCount = 1;
			write_set.dstBinding = layoutBinding.binding;
		}

		for (auto& [binding, imageSampler] : shader_descriptor_set.ImageSamplers) {
			auto& layoutBinding = layout_bindings.emplace_back();
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			layoutBinding.descriptorCount = imageSampler.ArraySize;
			layoutBinding.stageFlags = imageSampler.ShaderStage;
			layoutBinding.pImmutableSamplers = nullptr;
			layoutBinding.binding = binding;

			ensure(!shader_descriptor_set.UniformBuffers.contains(binding), "Binding is already present!");
			ensure(!shader_descriptor_set.StorageBuffers.contains(binding), "Binding is already present!");

			VkWriteDescriptorSet& write_set = shader_descriptor_set.WriteDescriptorSets[imageSampler.Name];
			write_set = {};
			write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_set.descriptorType = layoutBinding.descriptorType;
			write_set.descriptorCount = imageSampler.ArraySize;
			write_set.dstBinding = layoutBinding.binding;
		}

		for (auto& [binding, imageSampler] : shader_descriptor_set.SeparateTextures) {
			auto& layoutBinding = layout_bindings.emplace_back();
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			layoutBinding.descriptorCount = imageSampler.ArraySize;
			layoutBinding.stageFlags = imageSampler.ShaderStage;
			layoutBinding.pImmutableSamplers = nullptr;
			layoutBinding.binding = binding;

			ensure(!shader_descriptor_set.UniformBuffers.contains(binding), "Binding is already present!");
			ensure(!shader_descriptor_set.ImageSamplers.contains(binding), "Binding is already present!");
			ensure(!shader_descriptor_set.StorageBuffers.contains(binding), "Binding is already present!");

			VkWriteDescriptorSet& write_set = shader_descriptor_set.WriteDescriptorSets[imageSampler.Name];
			write_set = {};
			write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_set.descriptorType = layoutBinding.descriptorType;
			write_set.descriptorCount = imageSampler.ArraySize;
			write_set.dstBinding = layoutBinding.binding;
		}

		for (auto& [binding, imageSampler] : shader_descriptor_set.SeparateSamplers) {
			auto& layoutBinding = layout_bindings.emplace_back();
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
			layoutBinding.descriptorCount = imageSampler.ArraySize;
			layoutBinding.stageFlags = imageSampler.ShaderStage;
			layoutBinding.pImmutableSamplers = nullptr;
			layoutBinding.binding = binding;

			ensure(!shader_descriptor_set.UniformBuffers.contains(binding), "Binding is already present!");
			ensure(!shader_descriptor_set.ImageSamplers.contains(binding), "Binding is already present!");
			ensure(!shader_descriptor_set.StorageBuffers.contains(binding), "Binding is already present!");
			ensure(!shader_descriptor_set.SeparateTextures.contains(binding), "Binding is already present!");

			VkWriteDescriptorSet& write_set = shader_descriptor_set.WriteDescriptorSets[imageSampler.Name];
			write_set = {};
			write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_set.descriptorType = layoutBinding.descriptorType;
			write_set.descriptorCount = imageSampler.ArraySize;
			write_set.dstBinding = layoutBinding.binding;
		}

		for (auto& [bindingAndSet, imageSampler] : shader_descriptor_set.StorageImages) {
			auto& layoutBinding = layout_bindings.emplace_back();
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			layoutBinding.descriptorCount = imageSampler.ArraySize;
			layoutBinding.stageFlags = imageSampler.ShaderStage;
			layoutBinding.pImmutableSamplers = nullptr;

			uint32_t binding = bindingAndSet & 0xffffffff;
			// uint32_t descriptorSet = (bindingAndSet >> 32);
			layoutBinding.binding = binding;

			ensure(!shader_descriptor_set.UniformBuffers.contains(binding), "Binding is already present!");
			ensure(!shader_descriptor_set.StorageBuffers.contains(binding), "Binding is already present!");
			ensure(!shader_descriptor_set.ImageSamplers.contains(binding), "Binding is already present!");
			ensure(!shader_descriptor_set.SeparateTextures.contains(binding), "Binding is already present!");
			ensure(!shader_descriptor_set.SeparateSamplers.contains(binding), "Binding is already present!");

			VkWriteDescriptorSet& write_set = shader_descriptor_set.WriteDescriptorSets[imageSampler.Name];
			write_set = {};
			write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_set.descriptorType = layoutBinding.descriptorType;
			write_set.descriptorCount = 1;
			write_set.dstBinding = layoutBinding.binding;
		}

		VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
		descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorLayout.pNext = nullptr;
		descriptorLayout.bindingCount = static_cast<std::uint32_t>(layout_bindings.size());
		descriptorLayout.pBindings = layout_bindings.data();

		Log::info("Renderer",
			"Creating descriptor set {0} with {1} ubo's, {2} ssbo's, {3} samplers, {4} separate textures, {5} separate samplers and {6} storage ",
			set, shader_descriptor_set.UniformBuffers.size(), shader_descriptor_set.StorageBuffers.size(), shader_descriptor_set.ImageSamplers.size(),
			shader_descriptor_set.SeparateTextures.size(), shader_descriptor_set.SeparateSamplers.size(), shader_descriptor_set.StorageImages.size());
		if (set >= descriptor_set_layouts.size()) {
			descriptor_set_layouts.resize(static_cast<std::size_t>(set) + 1);
		}
		verify(vkCreateDescriptorSetLayout(vk_device, &descriptorLayout, nullptr, &descriptor_set_layouts[set]));
	}
}

} // namespace Disarray::Vulkan
