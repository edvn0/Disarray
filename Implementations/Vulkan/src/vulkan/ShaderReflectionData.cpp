#include "DisarrayPCH.hpp"

#include <spirv_cross.hpp>

#include <magic_enum.hpp>

#include "core/Ensure.hpp"
#include "vulkan/ShaderReflectionData.hpp"

namespace Disarray::Vulkan {

namespace {
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
			Log::error("ShaderReflectionData", "Unknown type for {}", magic_enum::enum_name(type.basetype));
			ensure(false);
			return Reflection::ShaderUniformType::None;
		}
		}
		ensure(false, "Unknown type!");
		return Reflection::ShaderUniformType::None;
	}

	std::unordered_map<std::uint32_t, std::unordered_map<std::uint32_t, Reflection::UniformBuffer>> global_uniform_buffers {};
	std::unordered_map<std::uint32_t, std::unordered_map<std::uint32_t, Reflection::StorageBuffer>> global_storage_buffers {};
} // namespace

auto reflect_code(VkShaderStageFlagBits shader_stage, const std::vector<std::uint32_t>& spirv, ReflectionData& output) -> void
{
	spirv_cross::Compiler compiler(spirv);
	auto resources = compiler.get_shader_resources();

	for (const auto& resource : resources.stage_inputs) {
		uint32_t location = compiler.get_decoration(resource.id, spv::DecorationLocation);
		const auto& type = compiler.get_type(resource.type_id);
		const std::string& name = compiler.get_name(resource.id);

		output.shader_inputs_outputs[shader_stage][Reflection::ShaderInputOrOutput::Input].push_back({
			location,
			name,
			spir_type_to_shader_uniform_type(type),
		});
	}

	for (const auto& resource : resources.stage_outputs) {
		uint32_t location = compiler.get_decoration(resource.id, spv::DecorationLocation);
		const auto& type = compiler.get_type(resource.type_id);
		const std::string& name = compiler.get_name(resource.id);

		output.shader_inputs_outputs[shader_stage][Reflection::ShaderInputOrOutput::Output].push_back({
			location,
			name,
			spir_type_to_shader_uniform_type(type),
		});
	}

	for (const auto& resource : resources.uniform_buffers) {
		if (auto active_buffers = compiler.get_active_buffer_ranges(resource.id); active_buffers.empty()) {
			continue;
		}

		const auto& name = resource.name;
		const auto& buffer_type = compiler.get_type(resource.base_type_id);
		auto binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
		auto descriptor_set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
		auto size = static_cast<std::uint32_t>(compiler.get_declared_struct_size(buffer_type));

		if (descriptor_set >= output.shader_descriptor_sets.size()) {
			output.shader_descriptor_sets.resize(descriptor_set + 1);
		}

		auto& shader_descriptor_set = output.shader_descriptor_sets[descriptor_set];
		if (!global_uniform_buffers[descriptor_set].contains(binding)) {
			Reflection::UniformBuffer uniform_buffer;
			uniform_buffer.binding_point = binding;
			uniform_buffer.size = size;
			uniform_buffer.name = name;
			uniform_buffer.shader_stage = VK_SHADER_STAGE_ALL_GRAPHICS;
			global_uniform_buffers.at(descriptor_set)[binding] = uniform_buffer;
		} else {
			if (auto& uniform_buffer = global_uniform_buffers.at(descriptor_set).at(binding); size > uniform_buffer.size) {
				uniform_buffer.size = size;
			}
		}
		shader_descriptor_set.uniform_buffers[binding] = global_uniform_buffers.at(descriptor_set).at(binding);
	}

	for (const auto& resource : resources.storage_buffers) {
		if (auto active_buffers = compiler.get_active_buffer_ranges(resource.id); active_buffers.empty()) {
			continue;
		}

		const auto& name = resource.name;
		const auto& buffer_type = compiler.get_type(resource.base_type_id);
		// auto member_count = static_cast<std::uint32_t>(buffer_type.member_types.size());
		auto binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
		auto descriptor_set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
		auto size = static_cast<std::uint32_t>(compiler.get_declared_struct_size(buffer_type));

		if (descriptor_set >= output.shader_descriptor_sets.size()) {
			output.shader_descriptor_sets.resize(descriptor_set + 1);
		}

		auto& shader_descriptor_set = output.shader_descriptor_sets[descriptor_set];
		if (!global_storage_buffers[descriptor_set].contains(binding)) {
			Reflection::StorageBuffer storage_buffer;
			storage_buffer.binding_point = binding;
			storage_buffer.size = size;
			storage_buffer.name = name;
			storage_buffer.shader_stage = VK_SHADER_STAGE_ALL_GRAPHICS;
			global_storage_buffers.at(descriptor_set)[binding] = storage_buffer;
		} else {
			if (auto& storage_buffer = global_storage_buffers.at(descriptor_set).at(binding); size > storage_buffer.size) {
				storage_buffer.size = size;
			}
		}

		shader_descriptor_set.storage_buffers[binding] = global_storage_buffers.at(descriptor_set).at(binding);
	}

	Log::trace("ShaderReflectionData", "Push Constant Buffers:");
	for (const auto& resource : resources.push_constant_buffers) {

		const auto& buffer_name = resource.name;
		if (output.constant_buffers.contains(buffer_name))
			continue;

		auto& buffer_type = compiler.get_type(resource.base_type_id);
		auto buffer_size = static_cast<std::uint32_t>(compiler.get_declared_struct_size(buffer_type));
		std::uint32_t member_count = static_cast<std::uint32_t>(buffer_type.member_types.size());
		auto& push_constant_range = output.push_constant_ranges.emplace_back();
		push_constant_range.shader_stage = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		push_constant_range.size = buffer_size;
		push_constant_range.offset = 0;

		if (buffer_name.empty())
			continue;

		auto& buffer = output.constant_buffers[buffer_name];
		buffer.name = buffer_name;
		buffer.size = buffer_size;

		for (std::uint32_t i = 0; i < member_count; i++) {
			auto type = compiler.get_type(buffer_type.member_types[i]);
			const auto& memberName = compiler.get_member_name(buffer_type.self, i);
			auto size = static_cast<uint32_t>(compiler.get_declared_struct_member_size(buffer_type, i));
			auto offset = compiler.type_struct_member_offset(buffer_type, i);

			std::string uniform_name = fmt::format("{}.{}", buffer_name, memberName);
			buffer.uniforms[uniform_name] = Reflection::ShaderUniform(uniform_name, spir_type_to_shader_uniform_type(type), size, offset);
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
		if (descriptor_set >= output.shader_descriptor_sets.size()) {
			output.shader_descriptor_sets.resize(descriptor_set + 1);
		}

		auto& shader_descriptor_set = output.shader_descriptor_sets[descriptor_set];
		auto& image_sampler = shader_descriptor_set.sampled_images[binding];
		image_sampler.binding_point = binding;
		image_sampler.descriptor_set = descriptor_set;
		image_sampler.name = name;
		image_sampler.shader_stage = shader_stage;
		image_sampler.array_size = array_size;

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
		if (descriptor_set >= output.shader_descriptor_sets.size()) {
			output.shader_descriptor_sets.resize(descriptor_set + 1);
		}

		auto& shader_descriptor_set = output.shader_descriptor_sets[descriptor_set];
		auto& image_sampler = shader_descriptor_set.separate_textures[binding];
		image_sampler.binding_point = binding;
		image_sampler.descriptor_set = descriptor_set;
		image_sampler.name = name;
		image_sampler.shader_stage = shader_stage;
		image_sampler.array_size = array_size;

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
		if (descriptor_set >= output.shader_descriptor_sets.size()) {
			output.shader_descriptor_sets.resize(descriptor_set + 1);
		}

		auto& shader_descriptor_set = output.shader_descriptor_sets[descriptor_set];
		auto& image_sampler = shader_descriptor_set.separate_samplers[binding];
		image_sampler.binding_point = binding;
		image_sampler.descriptor_set = descriptor_set;
		image_sampler.name = name;
		image_sampler.shader_stage = shader_stage;
		image_sampler.array_size = array_size;

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
		if (descriptor_set >= output.shader_descriptor_sets.size()) {
			output.shader_descriptor_sets.resize(descriptor_set + 1);
		}

		auto& shader_descriptor_set = output.shader_descriptor_sets[descriptor_set];
		auto& image_sampler = shader_descriptor_set.storage_images[binding];
		image_sampler.binding_point = binding;
		image_sampler.descriptor_set = descriptor_set;
		image_sampler.name = name;
		image_sampler.array_size = array_size;
		image_sampler.shader_stage = shader_stage;

		output.resources[name] = Reflection::ShaderResourceDeclaration(name, binding, 1);
	}
}

} // namespace Disarray::Vulkan
