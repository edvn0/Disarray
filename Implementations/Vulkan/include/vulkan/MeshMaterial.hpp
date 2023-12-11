#pragma once

#include "ShaderReflectionData.hpp"
#include "SingleShader.hpp"
#include "core/Ensure.hpp"
#include "graphics/MeshMaterial.hpp"
#include "graphics/Swapchain.hpp"
#include "vulkan/Texture.hpp"

namespace Disarray::Vulkan {

class MeshMaterial final : public Disarray::MeshMaterial {
public:
	MeshMaterial(const Disarray::Device& dev, MeshMaterialProperties properties);
	~MeshMaterial() override = default;

	auto recreate(bool should_clean, const Extent& extent) -> void override { recreate_material(should_clean, extent); }

	auto get_uniform_storage_buffer() const -> const DataBuffer& override { return uniform_storage_buffer; };

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
		if (!decl) {
			return;
		}

		auto& buffer = uniform_storage_buffer;
		buffer.write(Disarray::bit_cast<std::byte*>(&value), decl->get_size(), decl->get_offset());
	}

	template <typename T> auto get(const std::string& name) -> T&
	{
		const auto* decl = find_uniform_declaration(name);
		ensure(decl != nullptr, "Could not find uniform with name 'x'");
		const auto& buffer = uniform_storage_buffer;
		return buffer.read<T>(decl->get_offset());
	}

	template <typename T> auto get_resource(const std::string& name) -> Ref<T>
	{
		const auto* decl = find_resource_declaration(name);
		ensure(decl != nullptr, "Could not find uniform with name 'x'");
		const std::uint32_t slot = decl->get_register();
		ensure(slot < textures.size(), "Texture slot is invalid!");
		return Ref<T>(textures[slot]);
	}

	template <typename T> auto try_get_resource(const std::string& name) -> Ref<T>
	{
		const auto* decl = find_resource_declaration(name);
		if (!decl) {
			return nullptr;
		}

		const std::uint32_t slot = decl->get_register();
		if (slot >= textures.size()) {
			return nullptr;
		}

		return Ref<T>(textures[slot]);
	}

	auto get_descriptor_set(FrameIndex index) const -> VkDescriptorSet
	{
		return !descriptor_sets.at(index).descriptor_sets.empty() ? descriptor_sets.at(index).descriptor_sets[0] : nullptr;
	}

	auto update_for_rendering(FrameIndex frame_index, const std::vector<std::vector<VkWriteDescriptorSet>>&) -> void;
	auto update_for_rendering(const FrameIndex frame_index) -> void { update_for_rendering(frame_index, {}); }

private:
	auto clean_material() -> void;
	auto allocate_buffer_storage() -> void;
	auto recreate_material(bool should_clean, const Extent& extent) -> void;
	auto find_uniform_declaration(const std::string& name) -> const Reflection::ShaderUniform*;
	auto find_resource_declaration(const std::string& name) -> const Reflection::ShaderResourceDeclaration*;

	auto invalidate_descriptor_sets() -> void;
	auto invalidate() -> void;

	auto set_vulkan_descriptor(const std::string&, const Ref<Vulkan::Texture>&) -> void;
	auto set_vulkan_descriptor(const std::string&, const Ref<Vulkan::Texture3D>&) -> void;
	auto set_vulkan_descriptor(const std::string&, const Ref<Vulkan::Texture>&, std::uint32_t) -> void;
	auto set_vulkan_descriptor(const std::string&, const Ref<Vulkan::Image>& image) -> void;

	enum class PendingDescriptorType : std::uint8_t {
		None = 0,
		Texture2D = 1,
		TextureCube = 2,
		Image2D = 3,
	};

	struct PendingDescriptor {
		PendingDescriptorType type = PendingDescriptorType::None;
		VkWriteDescriptorSet write_set;
		VkDescriptorImageInfo image_info;
		Ref<Disarray::Texture> texture;
		Ref<Vulkan::Image> image;
		VkDescriptorImageInfo descriptor_image_info {};
	};

	struct PendingDescriptorArray {
		PendingDescriptorType type = PendingDescriptorType::None;
		VkWriteDescriptorSet write_set;
		std::vector<VkDescriptorImageInfo> image_infos;
		std::vector<Ref<Vulkan::Texture>> textures;
		std::vector<Ref<Vulkan::Image>> images;
		VkDescriptorImageInfo descriptor_image_info {};
	};
	std::unordered_map<uint32_t, std::shared_ptr<PendingDescriptor>> resident_descriptors;
	std::unordered_map<uint32_t, std::shared_ptr<PendingDescriptorArray>> resident_descriptor_arrays;
	std::vector<std::shared_ptr<PendingDescriptor>> pending_descriptors;

	DataBuffer uniform_storage_buffer;

	std::vector<Ref<Disarray::Texture>> textures;
	std::vector<std::vector<Ref<Vulkan::Texture>>> texture_arrays;
	std::vector<Ref<Vulkan::Image>> images;

	std::unordered_map<FrameIndex, MaterialDescriptorSet> descriptor_sets {};

	std::vector<std::vector<VkWriteDescriptorSet>> write_descriptors;
	std::vector<bool> dirty_descriptor_sets;

	Ref<Vulkan::SingleShader> shader { nullptr };
	const Disarray::Device& device;
};
} // namespace Disarray::Vulkan
