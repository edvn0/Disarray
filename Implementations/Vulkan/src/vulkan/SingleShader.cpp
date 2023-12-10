#include "vulkan/SingleShader.hpp"

#include <SPIRV/GlslangToSpv.h>
#include <vulkan/vulkan_core.h>

#include <glslang/Public/ShaderLang.h>

#include <ranges>

#include "core/String.hpp"
#include "core/filesystem/AssetLocations.hpp"
#include "core/filesystem/FileIO.hpp"
#include "graphics/SingleShader.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/GraphicsResource.hpp"

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

	auto to_stage(const ShaderType type)
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
} // namespace

class UnifiedIncluder final : public glslang::TShader::Includer {
public:
	explicit UnifiedIncluder(std::filesystem::path dir);
	~UnifiedIncluder() override;

	auto includeSystem(const char*, const char*, std::size_t) -> IncludeResult* override;
	auto includeLocal(const char*, const char*, std::size_t) -> IncludeResult* override;
	void releaseInclude(IncludeResult*) override;

private:
	auto get_or_emplace_include(const std::string& header_name) -> const std::string&;

	Collections::StringMap<std::string> sources {};
	std::filesystem::path directory;
};

UnifiedIncluder::UnifiedIncluder(std::filesystem::path dir)
	: directory(std::move(dir))
{
}

UnifiedIncluder::~UnifiedIncluder() = default;

auto UnifiedIncluder::includeSystem(const char* header_name, const char*, std::size_t) -> UnifiedIncluder::IncludeResult*
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

auto UnifiedIncluder::includeLocal(const char* header_name, const char*, std::size_t) -> UnifiedIncluder::IncludeResult*
{
	const auto& source = get_or_emplace_include(header_name);
	return new IncludeResult { header_name, source.c_str(), source.size(), nullptr };
}

void UnifiedIncluder::releaseInclude(IncludeResult* result) { delete result; }

SingleShader::SingleShader(const Disarray::Device& dev, SingleShaderProperties properties)
	: Disarray::SingleShader(std::move(properties))
	, device(dev)
	, name(props.path.filename().string())
{
	recreate_shader(false);
}

auto SingleShader::clean_shader() -> void
{
	sources.clear();
	spirv_sources.clear();
	for (const auto& module : shader_modules | std::views::values) {
		vkDestroyShaderModule(supply_cast<Vulkan::Device>(device), module, nullptr);
	}
	for (const auto& layout : descriptor_set_layouts) {
		vkDestroyDescriptorSetLayout(supply_cast<Vulkan::Device>(device), layout, nullptr);
	}
	shader_modules.clear();
	stage_infos.clear();
}

auto SingleShader::recreate_shader(bool should_clean) -> void
{
	if (should_clean) {
		clean_shader();
	}
	std::string read_file;
	if (!FS::read_from_file(props.path.string(), read_file)) {
		Log::error("SingleShader", "Failed to read shader file: {}", props.path.string());
		return;
	}
	using namespace std::string_view_literals;
	static constexpr auto vertex_delimiter = "#pragma stage vertex"sv;
	static constexpr auto fragment_delimiter = "#pragma stage fragment"sv;
	static constexpr auto vertex_delimiter_size = vertex_delimiter.size();

	auto split = StringUtilities::split_string(read_file, std::string { fragment_delimiter });

	auto& vertex = split.at(0);
	vertex.erase(0, vertex_delimiter_size);
	auto& fragment = split.at(1);

	sources[ShaderType::Vertex] = vertex;
	sources[ShaderType::Fragment] = fragment;

	// We now need a includer for the shader compiler
	UnifiedIncluder includer { "Assets/Shaders/Include" };

	glslang::TProgram program;
	std::unordered_map<ShaderType, Scope<glslang::TShader>> shaders {};
	for (auto&& [type, source] : sources) {
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

	for (auto&& [type, shader] : shaders) {
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

SingleShader::~SingleShader() { clean_shader(); }

// Hash combining function
template <typename T> void hash_combine(std::size_t& seed, const T& value)
{
	seed ^= std::hash<T> {}(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

// Hash function for vector of any type
template <typename T> struct VectorHash {
	std::size_t operator()(const std::vector<T>& vec) const
	{
		std::size_t hash = 0;
		for (const auto& elem : vec) {
			hash_combine(hash, elem);
		}
		return hash;
	}
};

// Hash function for unordered_map of ShaderType to vector
template <typename T> struct UnorderedMapHash {
	std::size_t operator()(const std::unordered_map<Disarray::ShaderType, std::vector<T>>& map) const
	{
		std::size_t hash = 0;
		for (const auto& pair : map) {
			hash_combine(hash, static_cast<std::size_t>(pair.first));
			hash_combine(hash, VectorHash<T> {}(pair.second));
		}
		return hash;
	}
};

auto reflect_on_stages(const std::unordered_map<Disarray::ShaderType, std::vector<std::uint32_t>>& spirv_codes) -> ReflectionData
{
	ReflectionData output {};

	const auto hash = UnorderedMapHash<std::uint32_t> {}(spirv_codes);
	static std::unordered_map<std::size_t, ReflectionData> cache {};
	if (cache.contains(hash)) {
		return cache.at(hash);
	}

	for (auto&& [type, stage_info] : spirv_codes) {
		reflect_code(to_stage(type), stage_info, output);
	}

	cache.emplace(hash, output);

	return output;
}

auto SingleShader::create_vulkan_objects() -> void
{
	for (auto&& [type, spirv] : spirv_sources) {
		VkShaderModuleCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.codeSize = spirv.size() * sizeof(std::uint32_t);
		create_info.pCode = spirv.data();

		VkShaderModule shader_module;
		if (vkCreateShaderModule(supply_cast<Vulkan::Device>(device), &create_info, nullptr, &shader_module) != VK_SUCCESS) {
			Log::error("SingleShader", "Failed to create shader module!");
			throw CouldNotCompileShaderException { "Failed to create shader module!" };
		}

		shader_modules.emplace(type, shader_module);
		stage_infos.try_emplace(type,
			VkPipelineShaderStageCreateInfo {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.pNext = nullptr,
				.stage = to_vulkan_shader_type(type),
				.module = shader_modules.at(type),
				.pName = "main",
				.pSpecializationInfo = nullptr,
			});
	}

	reflection_data = reflect_on_stages(spirv_sources);
	create_descriptor_set_layouts();
}

auto SingleShader::get_descriptor_set(const std::string& descriptor_name, std::uint32_t set) const -> const VkWriteDescriptorSet*
{
	if (set >= reflection_data.shader_descriptor_sets.size()) {
		return nullptr;
	}

	const auto& shader_descriptor_set = reflection_data.shader_descriptor_sets[set];
	if (shader_descriptor_set.write_descriptor_sets.contains(descriptor_name)) {
		return &shader_descriptor_set.write_descriptor_sets.at(descriptor_name);
	}

	Log::warn("Shader", "Shader {0} does not contain requested descriptor set {1}", props.path.string(), descriptor_name);
	return nullptr;
}

auto SingleShader::allocate_descriptor_set(std::uint32_t set) const -> MaterialDescriptorSet
{
	MaterialDescriptorSet result;

	if (reflection_data.shader_descriptor_sets.empty()) {
		return result;
	}

	VkDescriptorSetAllocateInfo allocation_info = {};
	allocation_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocation_info.descriptorSetCount = 1;
	allocation_info.pSetLayouts = &descriptor_set_layouts[set];
	auto& allocated_set = result.descriptor_sets.emplace_back();
	GraphicsResource::allocate_descriptor_sets(allocation_info, allocated_set);
	return result;
}

auto SingleShader::create_descriptor_set_layouts() -> void
{
	auto* vk_device = supply_cast<Vulkan::Device>(device);

	for (std::uint32_t set = 0; set < reflection_data.shader_descriptor_sets.size(); set++) {
		auto& shader_descriptor_set = reflection_data.shader_descriptor_sets[set];

		std::vector<VkDescriptorSetLayoutBinding> layout_bindings {};
		for (auto& [binding, uniform_buffer] : shader_descriptor_set.uniform_buffers) {
			VkDescriptorSetLayoutBinding& layout_binding = layout_bindings.emplace_back();
			layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			layout_binding.descriptorCount = 1;
			layout_binding.stageFlags = uniform_buffer.shader_stage;
			layout_binding.pImmutableSamplers = nullptr;
			layout_binding.binding = binding;

			VkWriteDescriptorSet& write_set = shader_descriptor_set.write_descriptor_sets[uniform_buffer.name];
			write_set = {};
			write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_set.descriptorType = layout_binding.descriptorType;
			write_set.descriptorCount = 1;
			write_set.dstBinding = layout_binding.binding;
		}

		for (auto& [binding, storage_buffer] : shader_descriptor_set.storage_buffers) {
			VkDescriptorSetLayoutBinding& layout_binding = layout_bindings.emplace_back();
			layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			layout_binding.descriptorCount = 1;
			layout_binding.stageFlags = storage_buffer.shader_stage;
			layout_binding.pImmutableSamplers = nullptr;
			layout_binding.binding = binding;
			ensure(!shader_descriptor_set.uniform_buffers.contains(binding), "Binding is already present!");

			VkWriteDescriptorSet& write_set = shader_descriptor_set.write_descriptor_sets[storage_buffer.name];
			write_set = {};
			write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_set.descriptorType = layout_binding.descriptorType;
			write_set.descriptorCount = 1;
			write_set.dstBinding = layout_binding.binding;
		}

		for (auto& [binding, image_sampler] : shader_descriptor_set.sampled_images) {
			auto& layout_binding = layout_bindings.emplace_back();
			layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			layout_binding.descriptorCount = image_sampler.array_size;
			layout_binding.stageFlags = image_sampler.shader_stage;
			layout_binding.pImmutableSamplers = nullptr;
			layout_binding.binding = binding;

			ensure(!shader_descriptor_set.uniform_buffers.contains(binding), "Binding is already present!");
			ensure(!shader_descriptor_set.storage_buffers.contains(binding), "Binding is already present!");

			VkWriteDescriptorSet& write_set = shader_descriptor_set.write_descriptor_sets[image_sampler.name];
			write_set = {};
			write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_set.descriptorType = layout_binding.descriptorType;
			write_set.descriptorCount = image_sampler.array_size;
			write_set.dstBinding = layout_binding.binding;
		}

		for (auto& [binding, image_sampler] : shader_descriptor_set.separate_textures) {
			auto& layout_binding = layout_bindings.emplace_back();
			layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			layout_binding.descriptorCount = image_sampler.array_size;
			layout_binding.stageFlags = image_sampler.shader_stage;
			layout_binding.pImmutableSamplers = nullptr;
			layout_binding.binding = binding;

			ensure(!shader_descriptor_set.uniform_buffers.contains(binding), "Binding is already present!");
			ensure(!shader_descriptor_set.sampled_images.contains(binding), "Binding is already present!");
			ensure(!shader_descriptor_set.storage_buffers.contains(binding), "Binding is already present!");

			VkWriteDescriptorSet& write_set = shader_descriptor_set.write_descriptor_sets[image_sampler.name];
			write_set = {};
			write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_set.descriptorType = layout_binding.descriptorType;
			write_set.descriptorCount = image_sampler.array_size;
			write_set.dstBinding = layout_binding.binding;
		}

		for (auto& [binding, image_sampler] : shader_descriptor_set.separate_samplers) {
			auto& layout_binding = layout_bindings.emplace_back();
			layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
			layout_binding.descriptorCount = image_sampler.array_size;
			layout_binding.stageFlags = image_sampler.shader_stage;
			layout_binding.pImmutableSamplers = nullptr;
			layout_binding.binding = binding;

			ensure(!shader_descriptor_set.uniform_buffers.contains(binding), "Binding is already present!");
			ensure(!shader_descriptor_set.sampled_images.contains(binding), "Binding is already present!");
			ensure(!shader_descriptor_set.storage_buffers.contains(binding), "Binding is already present!");
			ensure(!shader_descriptor_set.separate_textures.contains(binding), "Binding is already present!");

			VkWriteDescriptorSet& write_set = shader_descriptor_set.write_descriptor_sets[image_sampler.name];
			write_set = {};
			write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_set.descriptorType = layout_binding.descriptorType;
			write_set.descriptorCount = image_sampler.array_size;
			write_set.dstBinding = layout_binding.binding;
		}

		for (auto& [bindingAndSet, imageSampler] : shader_descriptor_set.storage_images) {
			auto& layout_binding = layout_bindings.emplace_back();
			layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			layout_binding.descriptorCount = imageSampler.array_size;
			layout_binding.stageFlags = imageSampler.shader_stage;
			layout_binding.pImmutableSamplers = nullptr;

			// Name a variable which has the value 0xFFFFFFFF
			static constexpr auto max_set = std::numeric_limits<std::uint32_t>::max();

			std::uint32_t binding = bindingAndSet & max_set;
			// uint32_t descriptorSet = (bindingAndSet >> 32);
			layout_binding.binding = binding;

			ensure(!shader_descriptor_set.uniform_buffers.contains(binding), "Binding is already present!");
			ensure(!shader_descriptor_set.storage_buffers.contains(binding), "Binding is already present!");
			ensure(!shader_descriptor_set.sampled_images.contains(binding), "Binding is already present!");
			ensure(!shader_descriptor_set.separate_textures.contains(binding), "Binding is already present!");
			ensure(!shader_descriptor_set.separate_samplers.contains(binding), "Binding is already present!");

			VkWriteDescriptorSet& write_set = shader_descriptor_set.write_descriptor_sets[imageSampler.name];
			write_set = {};
			write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_set.descriptorType = layout_binding.descriptorType;
			write_set.descriptorCount = 1;
			write_set.dstBinding = layout_binding.binding;
		}

		Collections::sort(
			layout_bindings, [](const VkDescriptorSetLayoutBinding& a, const VkDescriptorSetLayoutBinding& b) { return a.binding < b.binding; });

		VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
		descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_layout.pNext = nullptr;
		descriptor_layout.bindingCount = static_cast<std::uint32_t>(layout_bindings.size());
		descriptor_layout.pBindings = layout_bindings.data();

		Log::info("Renderer",
			"Creating descriptor set {0} with {1} ubo's, {2} ssbo's, {3} samplers, {4} separate textures, {5} separate samplers and {6} storage ",
			set, shader_descriptor_set.uniform_buffers.size(), shader_descriptor_set.storage_buffers.size(),
			shader_descriptor_set.sampled_images.size(), shader_descriptor_set.separate_textures.size(),
			shader_descriptor_set.separate_samplers.size(), shader_descriptor_set.storage_images.size());
		if (set >= descriptor_set_layouts.size()) {
			descriptor_set_layouts.resize(static_cast<std::size_t>(set) + 1);
		}
		verify(vkCreateDescriptorSetLayout(vk_device, &descriptor_layout, nullptr, &descriptor_set_layouts[set]));
	}
}

} // namespace Disarray::Vulkan
