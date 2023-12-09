#pragma once

#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>

#include <cstdint>

#include "core/Collections.hpp"

namespace Disarray::Vulkan {

namespace Reflection {

	struct Uniform {
		std::uint32_t binding { 0 };
		std::uint32_t set { 0 };
	};

	enum class ShaderUniformType : std::uint8_t {
		None = 0,
		Bool,
		Int,
		UInt,
		Float,
		Vec2,
		Vec3,
		Vec4,
		Mat3,
		Mat4,
		IVec2,
		IVec3,
		IVec4,
	};

	class ShaderUniform {
	public:
		ShaderUniform() = default;
		ShaderUniform(std::string_view name_view, ShaderUniformType input_type, std::uint32_t input_size, std::uint32_t input_offset)
			: name(name_view)
			, type(input_type)
			, size(input_size)
			, offset(input_offset) {};

		[[nodiscard]] auto get_name() const -> const std::string& { return name; }
		[[nodiscard]] auto get_type() const -> ShaderUniformType { return type; }
		[[nodiscard]] auto get_size() const -> std::uint32_t { return size; }
		[[nodiscard]] auto get_offset() const -> std::uint32_t { return offset; }

	private:
		std::string name;
		ShaderUniformType type { ShaderUniformType::None };
		std::uint32_t size { 0 };
		std::uint32_t offset { 0 };
	};

	struct ShaderUniformBuffer {
		std::string name;
		std::uint32_t index { 0 };
		std::uint32_t binding_point { 0 };
		std::uint32_t size { 0 };
		std::vector<ShaderUniform> uniforms {};
	};

	struct ShaderStorageBuffer {
		std::string name;
		std::uint32_t index { 0 };
		std::uint32_t binding_point { 0 };
		std::uint32_t size { 0 };
		// std::vector<ShaderUniform> Uniforms;
	};

	struct ShaderBuffer {
		std::string name;
		std::uint32_t size { 0 };
		std::unordered_map<std::string, ShaderUniform> uniforms;
	};

	struct UniformBuffer {
		VkDescriptorBufferInfo descriptor {};
		std::uint32_t size { 0 };
		std::uint32_t binding_point { 0 };
		std::string name;
		VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	};

	struct StorageBuffer {
		VmaAllocation allocation = nullptr;
		VkDescriptorBufferInfo descriptor {};
		std::uint32_t size { 0 };
		std::uint32_t binding_point { 0 };
		std::string name;
		VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	};

	struct ImageSampler {
		std::uint32_t binding_point { 0 };
		std::uint32_t descriptor_set { 0 };
		std::uint32_t array_size { 0 };
		std::string name;
		VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	};

	struct PushConstantRange {
		VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		std::uint32_t offset { 0 };
		std::uint32_t size { 0 };
	};

	struct ShaderDescriptorSet {
		std::unordered_map<std::uint32_t, UniformBuffer> uniform_buffers {};
		std::unordered_map<std::uint32_t, StorageBuffer> storage_buffers {};
		std::unordered_map<std::uint32_t, ImageSampler> sampled_images {};
		std::unordered_map<std::uint32_t, ImageSampler> storage_images {};
		std::unordered_map<std::uint32_t, ImageSampler> separate_textures {};
		std::unordered_map<std::uint32_t, ImageSampler> separate_samplers {};

		std::unordered_map<std::string, VkWriteDescriptorSet> write_descriptor_sets {};
	};

	class ShaderResourceDeclaration final {
	public:
		~ShaderResourceDeclaration() = default;
		ShaderResourceDeclaration() = default;
		ShaderResourceDeclaration(std::string input_name, std::uint32_t reg, std::uint32_t input_count)
			: name(std::move(input_name))
			, resource_register(reg)
			, count(input_count)
		{
		}

		[[nodiscard]] auto get_name() const -> const std::string& { return name; }
		[[nodiscard]] auto get_register() const -> std::uint32_t { return resource_register; }
		[[nodiscard]] auto get_count() const -> std::uint32_t { return count; }

	private:
		std::string name;
		std::uint32_t resource_register { 0 };
		std::uint32_t count { 0 };
	};

	enum class ShaderInputOrOutput : std::uint8_t {
		Input,
		Output,
	};

	struct ShaderInOut {
		std::uint32_t location { 0 };
		std::string name;
		ShaderUniformType type;
	};
} // namespace Reflection

struct MaterialDescriptorSet {
	std::vector<VkDescriptorSet> descriptor_sets {};
};

struct ReflectionData {
	std::vector<Reflection::ShaderDescriptorSet> shader_descriptor_sets {};
	std::vector<Reflection::PushConstantRange> push_constant_ranges {};
	Collections::StringMap<Reflection::ShaderBuffer> constant_buffers {};
	Collections::StringMap<Reflection::ShaderResourceDeclaration> resources {};
	std::unordered_map<VkShaderStageFlagBits, std::unordered_map<Reflection::ShaderInputOrOutput, std::vector<Reflection::ShaderInOut>>>
		shader_inputs_outputs {};
};

auto reflect_code(VkShaderStageFlagBits shaderStage, const std::vector<std::uint32_t>& spirv, ReflectionData& output) -> void;

} // namespace Disarray::Vulkan
