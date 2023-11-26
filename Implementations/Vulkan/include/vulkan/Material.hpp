#pragma once

#include "Shader.hpp"
#include "graphics/Device.hpp"
#include "graphics/Material.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Texture.hpp"
#include "graphics/UniformBuffer.hpp"
#include "vulkan/GraphicsResource.hpp"

using VkDescriptorSet = struct VkDescriptorSet_T*;
using VkDescriptorSetLayout = struct VkDescriptorSetLayout_T*;

namespace Disarray::Vulkan {

class Material : public Disarray::Material {
public:
	Material(const Disarray::Device& dev, const MaterialProperties& properties);
	~Material() override;

	void recreate(bool, const Extent&) override;
	void force_recreation() override;

	void update_material(Disarray::Renderer&) override;
	void bind(const Disarray::CommandExecutor& executor, const Disarray::Pipeline& pipeline, FrameIndex index) const override;
	void write_textures(IGraphicsResource& resource) const override;

private:
	void recreate_material(bool should_clean);
	void recreate_descriptor_set_layout(Vulkan::GraphicsResource&);

	const Disarray::Device& device;

	std::vector<VkDescriptorSetLayout> layouts {};
	std::unordered_map<FrameIndex, std::vector<VkDescriptorSet>> frame_based_descriptor_sets {};
	bool needs_update { true };
};

class POCMaterial final : public Disarray::POCMaterial {
public:
	POCMaterial(const Disarray::Device& dev, POCMaterialProperties properties);
	~POCMaterial() override = default;

	auto recreate(bool should_clean, const Extent& extent) -> void override { recreate_material(should_clean, extent); }

	void set(const std::string&, float) override;
	void set(const std::string&, int) override;
	void set(const std::string&, std::uint32_t) override;
	void set(const std::string&, bool) override;
	void set(const std::string&, const glm::ivec2&) override;
	void set(const std::string&, const glm::ivec3&) override;
	void set(const std::string&, const glm::ivec4&) override;
	void set(const std::string&, const glm::uvec2&) override;
	void set(const std::string&, const glm::uvec3&) override;
	void set(const std::string&, const glm::uvec4&) override;
	void set(const std::string&, const glm::vec2&) override;
	void set(const std::string&, const glm::vec3&) override;
	void set(const std::string&, const glm::vec4&) override;
	void set(const std::string&, const glm::mat3&) override;
	void set(const std::string&, const glm::mat4&) override;
	void set(const std::string&, const Ref<Disarray::Texture>&) override;
	void set(const std::string&, const Ref<Disarray::Texture>&, std::uint32_t) override;
	void set(const std::string&, const Ref<Disarray::Image>& image) override;

	template <typename T> void set(const std::string& name, const T& value)
	{
		const auto* decl = find_uniform_declaration(name);
		ensure(decl != nullptr, "Could not find uniform!");
		if (!decl)
			return;

		auto& buffer = uniform_storage_buffer;
		buffer.write(Disarray::bit_cast<std::byte*>(&value), decl->get_size(), decl->get_offset());
	}

	template <typename T> T& get(const std::string& name)
	{
		const auto* decl = find_uniform_declaration(name);
		ensure(decl != nullptr, "Could not find uniform with name 'x'");
		const auto& buffer = uniform_storage_buffer;
		return buffer.read<T>(decl->get_offset());
	}

	template <typename T> Ref<T> get_resource(const std::string& name)
	{
		const auto* decl = find_resouce_declaration(name);
		ensure(decl != nullptr, "Could not find uniform with name 'x'");
		const std::uint32_t slot = decl->get_register();
		ensure(slot < textures.size(), "Texture slot is invalid!");
		return Ref<T>(textures[slot]);
	}

	template <typename T> Ref<T> try_get_resource(const std::string& name)
	{
		auto* decl = find_resouce_declaration(name);
		if (!decl)
			return nullptr;

		const std::uint32_t slot = decl->get_register();
		if (slot >= textures.size())
			return nullptr;

		return Ref<T>(textures[slot]);
	}

private:
	auto clean_material() -> void;
	auto allocate_buffer_storage() -> void;
	auto recreate_material(bool should_clean, const Extent& extent) -> void;
	auto find_uniform_declaration(const std::string& name) -> const Reflection::ShaderUniform*;
	auto find_resouce_declaration(const std::string& name) -> const Reflection::ShaderResourceDeclaration*;

	enum class PendingDescriptorType { None = 0, Texture2D, TextureCube, Image2D };
	struct PendingDescriptor {
		PendingDescriptorType type = PendingDescriptorType::None;
		VkWriteDescriptorSet write_set;
		VkDescriptorImageInfo image_info;
		Ref<Texture> texture;
		Ref<Image> image;
		VkDescriptorImageInfo descriptor_image_info {};
	};

	struct PendingDescriptorArray {
		PendingDescriptorType type = PendingDescriptorType::None;
		VkWriteDescriptorSet write_set;
		std::vector<VkDescriptorImageInfo> image_infos;
		std::vector<Ref<Texture>> textures;
		std::vector<Ref<Image>> images;
		VkDescriptorImageInfo descriptor_image_info {};
	};
	std::unordered_map<uint32_t, std::shared_ptr<PendingDescriptor>> resident_descriptors;
	std::unordered_map<uint32_t, std::shared_ptr<PendingDescriptorArray>> resident_descriptor_arrays;
	std::vector<std::shared_ptr<PendingDescriptor>> pending_descriptors;

	DataBuffer uniform_storage_buffer;

	std::vector<Ref<Texture>> textures;
	std::vector<std::vector<Ref<Texture>>> texture_arrays;
	std::vector<Ref<Image>> images;

	std::vector<VkDescriptorSet> descriptor_sets {};

	std::vector<std::vector<VkWriteDescriptorSet>> write_descriptors;
	std::vector<bool> dirty_descriptor_sets;

	const Disarray::Device& device;
};

} // namespace Disarray::Vulkan
