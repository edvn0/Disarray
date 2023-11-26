#pragma once

#include <vk_mem_alloc.h>

#include <utility>

#include "core/Collections.hpp"
#include "graphics/Shader.hpp"
#include "vulkan/PropertySupplier.hpp"

namespace Disarray::Vulkan {

namespace Reflection {

	struct Uniform {
		std::uint32_t binding { 0 };
		std::uint32_t set { 0 };
	};

	enum class ShaderUniformType { None = 0, Bool, Int, UInt, Float, Vec2, Vec3, Vec4, Mat3, Mat4, IVec2, IVec3, IVec4 };

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
		ShaderUniformType type = ShaderUniformType::None;
		std::uint32_t size = 0;
		std::uint32_t offset = 0;
	};

	struct ShaderUniformBuffer {
		std::string Name;
		std::uint32_t Index;
		std::uint32_t BindingPoint;
		std::uint32_t Size;
		std::uint32_t RendererID;
		std::vector<ShaderUniform> Uniforms;
	};

	struct ShaderStorageBuffer {
		std::string Name;
		std::uint32_t Index;
		std::uint32_t BindingPoint;
		std::uint32_t Size;
		std::uint32_t RendererID;
		// std::vector<ShaderUniform> Uniforms;
	};

	struct ShaderBuffer {
		std::string Name;
		std::uint32_t Size = 0;
		std::unordered_map<std::string, ShaderUniform> Uniforms;
	};

	struct UniformBuffer {
		VkDescriptorBufferInfo Descriptor;
		std::uint32_t Size = 0;
		std::uint32_t BindingPoint = 0;
		std::string Name;
		VkShaderStageFlagBits ShaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	};

	struct StorageBuffer {
		VmaAllocation MemoryAlloc = nullptr;
		VkDescriptorBufferInfo Descriptor;
		std::uint32_t Size = 0;
		std::uint32_t BindingPoint = 0;
		std::string Name;
		VkShaderStageFlagBits ShaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	};

	struct ImageSampler {
		std::uint32_t BindingPoint = 0;
		std::uint32_t DescriptorSet = 0;
		std::uint32_t ArraySize = 0;
		std::string Name;
		VkShaderStageFlagBits ShaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	};

	struct PushConstantRange {
		VkShaderStageFlagBits ShaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		std::uint32_t Offset = 0;
		std::uint32_t Size = 0;
	};

	struct ShaderDescriptorSet {
		std::unordered_map<std::uint32_t, UniformBuffer> UniformBuffers;
		std::unordered_map<std::uint32_t, StorageBuffer> StorageBuffers;
		std::unordered_map<std::uint32_t, ImageSampler> ImageSamplers;
		std::unordered_map<std::uint32_t, ImageSampler> StorageImages;
		std::unordered_map<std::uint32_t, ImageSampler> SeparateTextures;
		std::unordered_map<std::uint32_t, ImageSampler> SeparateSamplers;

		std::unordered_map<std::string, VkWriteDescriptorSet> WriteDescriptorSets;
	};

	class ShaderResourceDeclaration {
	public:
		ShaderResourceDeclaration() = default;
		ShaderResourceDeclaration(std::string input_name, std::uint32_t reg, std::uint32_t input_count)
			: name(std::move(input_name))
			, resource_register(reg)
			, count(input_count)
		{
		}
		~ShaderResourceDeclaration() = default;

		[[nodiscard]] virtual auto get_name() const -> const std::string& { return name; }
		[[nodiscard]] virtual auto get_register() const -> std::uint32_t { return resource_register; }
		[[nodiscard]] virtual auto get_count() const -> std::uint32_t { return count; }

	private:
		std::string name;
		std::uint32_t resource_register = 0;
		std::uint32_t count = 0;
	};
} // namespace Reflection

struct ReflectionData {
	std::vector<Reflection::ShaderDescriptorSet> ShaderDescriptorSets;
	std::vector<Reflection::PushConstantRange> PushConstantRanges;
	Collections::StringMap<Reflection::ShaderBuffer> constant_buffers {};
	Collections::StringMap<Reflection::ShaderResourceDeclaration> resources {};

	auto operator|=(const ReflectionData& other) -> ReflectionData&;
};

class Shader : public Disarray::Shader, public PropertySupplier<VkPipelineShaderStageCreateInfo> {
	DISARRAY_MAKE_NONCOPYABLE(Shader)
public:
	Shader(const Disarray::Device& device, ShaderProperties);
	Shader(const Disarray::Device& device, const std::filesystem::path&);
	~Shader() override;

	auto attachment_count() const -> std::uint32_t override;

	[[nodiscard]] auto supply() const -> VkPipelineShaderStageCreateInfo override { return stage; }
	[[nodiscard]] auto supply() -> VkPipelineShaderStageCreateInfo override { return stage; }

	void destroy_module() override;

	void create_descriptors();

private:
	static auto read_file(const std::filesystem::path&) -> std::string;

	bool was_destroyed_explicitly { false };

	ReflectionData reflection_data {};
	std::vector<VkDescriptorSetLayout> descriptor_set_layouts {};
	VkDescriptorSet descriptor_set { nullptr };

	std::unordered_map<uint32_t, std::vector<VkDescriptorPoolSize>> type_counts {};

	const Disarray::Device& device;
	VkPipelineShaderStageCreateInfo stage {};
	VkShaderModule shader_module {};
};
} // namespace Disarray::Vulkan
