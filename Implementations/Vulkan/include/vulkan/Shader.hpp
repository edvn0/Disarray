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
		ShaderUniform(std::string_view name, ShaderUniformType type, std::uint32_t size, std::uint32_t offset)
			: m_Name(name)
			, m_Type(type)
			, m_Size(size)
			, m_Offset(offset) {};

		const std::string& GetName() const { return m_Name; }
		ShaderUniformType GetType() const { return m_Type; }
		std::uint32_t GetSize() const { return m_Size; }
		std::uint32_t GetOffset() const { return m_Offset; }

	private:
		std::string m_Name;
		ShaderUniformType m_Type = ShaderUniformType::None;
		std::uint32_t m_Size = 0;
		std::uint32_t m_Offset = 0;
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
		ShaderResourceDeclaration(std::string name, std::uint32_t resourceRegister, std::uint32_t count)
			: m_Name(std::move(name))
			, m_Register(resourceRegister)
			, m_Count(count)
		{
		}
		~ShaderResourceDeclaration() = default;

		virtual const std::string& GetName() const { return m_Name; }
		virtual std::uint32_t GetRegister() const { return m_Register; }
		virtual std::uint32_t GetCount() const { return m_Count; }

	private:
		std::string m_Name;
		std::uint32_t m_Register = 0;
		std::uint32_t m_Count = 0;
	};
} // namespace Reflection

struct ReflectionData {
	std::vector<Reflection::ShaderDescriptorSet> ShaderDescriptorSets;
	std::vector<Reflection::PushConstantRange> PushConstantRanges;
	Collections::StringMap<Reflection::ShaderBuffer> constant_buffers {};
	Collections::StringMap<Reflection::ShaderResourceDeclaration> resources {};
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

private:
	static auto read_file(const std::filesystem::path&) -> std::string;

	bool was_destroyed_explicitly { false };

	ReflectionData reflection_data {};
	std::vector<VkDescriptorSetLayout> descriptor_set_layouts {};
	VkDescriptorSet descriptor_set { nullptr };

	const Disarray::Device& device;
	VkPipelineShaderStageCreateInfo stage {};
	VkShaderModule shader_module {};
};
} // namespace Disarray::Vulkan
