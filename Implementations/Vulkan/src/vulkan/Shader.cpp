#include "DisarrayPCH.hpp"

#include <spirv_reflect.hpp>

#include <fstream>

#include "core/Ensure.hpp"
#include "core/Formatters.hpp"
#include "graphics/Shader.hpp"
#include "graphics/ShaderCompiler.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Shader.hpp"
#include "vulkan/Structures.hpp"
#include "vulkan/Verify.hpp"

namespace Disarray::Vulkan {

namespace {

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

	void create_module(const Vulkan::Device& device, const std::string& spirv_code, VkShaderModule& shader)
	{
		auto create_info = vk_structures<VkShaderModuleCreateInfo> {}();
		create_info.codeSize = spirv_code.size() * sizeof(std::uint32_t);
		create_info.pCode = Disarray::bit_cast<const uint32_t*>(spirv_code.data());

		verify(vkCreateShaderModule(*device, &create_info, nullptr, &shader));
	}

	void create_module(const Vulkan::Device& device, const std::vector<std::uint32_t>& spirv_code, VkShaderModule& shader)
	{
		auto create_info = vk_structures<VkShaderModuleCreateInfo> {}();
		create_info.codeSize = spirv_code.size() * sizeof(std::uint32_t);
		create_info.pCode = spirv_code.data();

		verify(vkCreateShaderModule(*device, &create_info, nullptr, &shader));
	}

	auto reflect_code(VkShaderStageFlagBits shaderStage, std::vector<std::uint32_t>& spirv, ReflectionData& output)
	{
		const spirv_cross::Compiler compiler(spirv);
		auto resources = compiler.get_shader_resources();

		for (const auto& resource : resources.uniform_buffers) {
			auto active_buffers = compiler.get_active_buffer_ranges(resource.id);
			// Discard unused buffers from headers
			if (!active_buffers.empty()) {
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
		}

		for (const auto& resource : resources.storage_buffers) {
			auto active_buffers = compiler.get_active_buffer_ranges(resource.id);
			if (!active_buffers.empty()) {
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

Shader::Shader(const Disarray::Device& dev, ShaderProperties properties)
	: Disarray::Shader(std::move(properties))
	, device(dev)
{
	auto type = to_stage(props.type);

	if (props.code) {
		ensure(!props.identifier.empty(), "Must supply an identifier");
		reflect_code(type, *props.code, reflection_data);
		create_module(cast_to<Vulkan::Device>(device), *props.code, shader_module);
	} else {
		ensure(props.path.has_value(), "No code, but no path provided.");
		props.identifier = props.path.value();
		auto read = Shader::read_file(props.path.value());
		create_module(cast_to<Vulkan::Device>(device), read, shader_module);
	}

	stage = {};
	stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage.stage = type;
	stage.module = shader_module;
	stage.pName = props.entry_point.data();
}

Shader::Shader(const Disarray::Device& dev, const std::filesystem::path& path)
	: device(dev)
{
	static Runtime::ShaderCompiler compiler {};
	props.type = to_shader_type(path);
	props.path = path;
	auto type = to_stage(props.type);
	props.identifier = path;
	auto&& [could, code] = compiler.try_compile(path, props.type);
	if (!could) {
		Log::info("Shader", "Could not compile {}", path);
		throw CouldNotCompileShaderException { fmt::format("Path: {}", path) };
	}
	props.code = code;

	reflect_code(type, *props.code, reflection_data);

	create_module(cast_to<Vulkan::Device>(device), *props.code, shader_module);

	stage = {};
	stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage.stage = type;
	stage.module = shader_module;
	stage.pName = props.entry_point.data();
}

Shader::~Shader()
{
	if (!was_destroyed_explicitly) {
		vkDestroyShaderModule(supply_cast<Vulkan::Device>(device), shader_module, nullptr);
	}
}

auto Shader::read_file(const std::filesystem::path& path) -> std::string
{
	std::ifstream stream { path.string().c_str(), std::ios::ate | std::ios::in | std::ios::binary };
	if (!stream) {
		throw CouldNotOpenStreamException("Could not open stream to file");
	}

	const std::size_t size = stream.tellg();
	std::vector<char> buffer {};
	buffer.resize(size);

	stream.seekg(0);
	stream.read(buffer.data(), static_cast<long long>(size));

	return std::string { buffer.begin(), buffer.end() };
}

void Shader::destroy_module()
{
	vkDestroyShaderModule(supply_cast<Vulkan::Device>(device), shader_module, nullptr);
	was_destroyed_explicitly = true;
}

auto Shader::attachment_count() const -> std::uint32_t { return 0; }

} // namespace Disarray::Vulkan
