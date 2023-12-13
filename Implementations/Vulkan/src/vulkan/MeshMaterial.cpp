#include "DisarrayPCH.hpp"

#include <ranges>

#include "vulkan/Device.hpp"
#include "vulkan/MeshMaterial.hpp"
#include "vulkan/SingleShader.hpp"

namespace Disarray::Vulkan {

MeshMaterial::MeshMaterial(const Disarray::Device& dev, MeshMaterialProperties properties)
	: Disarray::MeshMaterial(std::move(properties))
	, write_descriptors(props.swapchain_image_count)
	, dirty_descriptor_sets(props.swapchain_image_count, false)
	, shader(props.shader.as<Vulkan::SingleShader>())
	, device(dev)
{
	recreate(false, {});
}

void MeshMaterial::set_vulkan_descriptor(const std::string& name, const Ref<Vulkan::Texture>& texture)
{
	const auto* resource = find_resource_declaration(name);
	const std::uint32_t binding = resource->get_register();

	if (binding < textures.size() && textures.at(binding) && texture->hash() == textures.at(binding)->hash()
		&& resident_descriptors.contains(binding)) {
		return;
	}

	if (binding >= textures.size()) {
		textures.resize(binding + 1);
	}
	textures.at(binding) = texture;

	const auto* wds = shader->get_descriptor_set(name);
	resident_descriptors[binding]
		= std::make_shared<PendingDescriptor>(PendingDescriptor { PendingDescriptorType::Texture2D, *wds, {}, texture, nullptr });
	pending_descriptors.push_back(resident_descriptors.at(binding));

	invalidate_descriptor_sets();
}

auto MeshMaterial::set_vulkan_descriptor(const std::string& name, const Ref<Vulkan::Texture3D>& texture) -> void
{
	const auto* resource = find_resource_declaration(name);

	const std::uint32_t binding = resource->get_register();
	// Texture is already set
	if (binding < textures.size() && textures.at(binding) && texture->hash() == textures.at(binding)->hash()
		&& resident_descriptors.contains(binding)) {
		return;
	}

	if (binding >= textures.size()) {
		textures.resize(binding + 1);
	}
	textures.at(binding) = texture;

	const auto* wds = shader->get_descriptor_set(name);
	resident_descriptors.at(binding)
		= std::make_shared<PendingDescriptor>(PendingDescriptor { PendingDescriptorType::TextureCube, *wds, {}, texture, nullptr });
	pending_descriptors.push_back(resident_descriptors.at(binding));

	invalidate_descriptor_sets();
}

void MeshMaterial::set_vulkan_descriptor(const std::string& name, const Ref<Vulkan::Texture>& texture, uint32_t array_index)
{
	const auto* resource = find_resource_declaration(name);

	const std::uint32_t binding = resource->get_register();
	// Texture is already set
	if (binding < texture_arrays.size() && texture_arrays.at(binding).size() < array_index
		&& texture->hash() == texture_arrays.at(binding).at(array_index)->hash()) {
		return;
	}

	if (binding >= texture_arrays.size()) {
		texture_arrays.resize(binding + 1);
	}

	if (array_index >= texture_arrays.at(binding).size()) {
		texture_arrays.at(binding).resize(array_index + 1);
	}

	texture_arrays.at(binding).at(array_index) = texture;

	const auto* wds = shader->get_descriptor_set(name);
	if (!resident_descriptor_arrays.contains(binding)) {
		resident_descriptor_arrays.at(binding)
			= std::make_shared<PendingDescriptorArray>(PendingDescriptorArray { PendingDescriptorType::Texture2D, *wds, {}, {}, {} });
	}

	const auto& resident_descriptor_array = resident_descriptor_arrays.at(binding);
	if (array_index >= resident_descriptor_array->textures.size()) {
		resident_descriptor_array->textures.resize(array_index + 1);
	}

	resident_descriptor_array->textures.at(array_index) = texture;

	invalidate_descriptor_sets();
}

void MeshMaterial::set_vulkan_descriptor(const std::string& name, const Ref<Vulkan::Image>& image)
{
	const auto* resource = find_resource_declaration(name);

	const std::uint32_t binding = resource->get_register();
	if (binding < images.size() && images.at(binding) && resident_descriptors.contains(binding)) {
		return;
	}

	if (resource->get_register() >= images.size()) {
		images.resize(resource->get_register() + 1);
	}
	images[resource->get_register()] = image;

	const auto* wds = shader->get_descriptor_set(name);
	resident_descriptors.at(binding)
		= std::make_shared<PendingDescriptor>(PendingDescriptor { PendingDescriptorType::Image2D, *wds, {}, nullptr, image });
	pending_descriptors.push_back(resident_descriptors.at(binding));

	invalidate_descriptor_sets();
}

void MeshMaterial::set(const std::string& name, const Ref<Disarray::Texture>& value)
{
	if (value->get_properties().dimension == Disarray::TextureDimension::Two) {
		set_vulkan_descriptor(name, value.as<Vulkan::Texture>());
	} else {
		set_vulkan_descriptor(name, value.as<Vulkan::Texture3D>());
	}
}

void MeshMaterial::set(const std::string& name, const Ref<Disarray::Texture>& texture, std::uint32_t value)
{
	set_vulkan_descriptor(name, texture.as<Vulkan::Texture>(), value);
}

auto MeshMaterial::set(const std::string& name, float value) -> void { set<>(name, value); }
auto MeshMaterial::set(const std::string& name, int value) -> void { set<>(name, value); }
auto MeshMaterial::set(const std::string& name, std::uint32_t value) -> void { set<>(name, value); }
auto MeshMaterial::set(const std::string& name, bool value) -> void { set<>(name, value); }
auto MeshMaterial::set(const std::string& name, const glm::ivec2& value) -> void { set<>(name, value); }
auto MeshMaterial::set(const std::string& name, const glm::ivec3& value) -> void { set<>(name, value); }
auto MeshMaterial::set(const std::string& name, const glm::ivec4& value) -> void { set<>(name, value); }
auto MeshMaterial::set(const std::string& name, const glm::uvec2& value) -> void { set<>(name, value); }
auto MeshMaterial::set(const std::string& name, const glm::uvec3& value) -> void { set<>(name, value); }
auto MeshMaterial::set(const std::string& name, const glm::uvec4& value) -> void { set<>(name, value); }
auto MeshMaterial::set(const std::string& name, const glm::vec2& value) -> void { set<>(name, value); }
auto MeshMaterial::set(const std::string& name, const glm::vec3& value) -> void { set<>(name, value); }
auto MeshMaterial::set(const std::string& name, const glm::vec4& value) -> void { set<>(name, value); }
auto MeshMaterial::set(const std::string& name, const glm::mat3& value) -> void { set<>(name, value); }
auto MeshMaterial::set(const std::string& name, const glm::mat4& value) -> void { set<>(name, value); }
auto MeshMaterial::set(const std::string& name, const Ref<Disarray::Image>& image) -> void { set_vulkan_descriptor(name, image.as<Vulkan::Image>()); }

auto MeshMaterial::clean_material() -> void
{
	resident_descriptors.clear();
	resident_descriptor_arrays.clear();
	descriptor_sets.clear();
}

auto MeshMaterial::allocate_buffer_storage() -> void
{
	if (const auto& shader_buffers = shader->get_shader_buffers(); !shader_buffers.empty()) {
		std::uint32_t size = 0;
		for (auto&& [id, buffer_size, buffer] : std::ranges::views::values(shader_buffers)) {
			size += buffer_size;
		}
		uniform_storage_buffer.allocate(size);
		uniform_storage_buffer.zero_initialise();
	}
}

auto MeshMaterial::recreate_material(bool should_clean, const Extent&) -> void
{
	if (should_clean) {
		clean_material();
	}

	allocate_buffer_storage();
	invalidate();
}

auto MeshMaterial::invalidate() -> void
{
	if (!shader->has_descriptor_set(0)) {
		return;
	}

	if (const auto& shader_descriptor_sets = shader->get_shader_descriptor_sets(); !shader_descriptor_sets.empty()) {
		for (auto& descriptor : resident_descriptors | std::views::values) {
			pending_descriptors.push_back(descriptor);
		}
	}
}

auto MeshMaterial::find_uniform_declaration(const std::string& name) -> const Reflection::ShaderUniform*
{
	const auto& shader_buffers = shader->get_shader_buffers();
	if (shader_buffers.empty()) {
		return nullptr;
	}

	const auto& [_, size, uniforms] = shader_buffers.begin()->second;
	if (!uniforms.contains(name)) {
		return nullptr;
	}

	return &uniforms.at(name);
}

auto MeshMaterial::find_resource_declaration(const std::string& name) -> const Reflection::ShaderResourceDeclaration*
{
	if (const auto& resources = shader->get_resources(); resources.contains(name)) {
		return &resources.at(name);
	}

	return nullptr;
}

auto MeshMaterial::invalidate_descriptor_sets() -> void
{
	for (auto i = FrameIndex { 0 }; i < props.swapchain_image_count; ++i) {
		dirty_descriptor_sets.at(i.value) = true;
	}
}

auto MeshMaterial::update_for_rendering(FrameIndex frame_index, const std::vector<std::vector<VkWriteDescriptorSet>>& uniform_write_descriptors)
	-> void
{
	auto* vk_device = supply_cast<Vulkan::Device>(device);
	for (const auto& descriptor : resident_descriptors | std::views::values) {
		if (descriptor->type == PendingDescriptorType::Image2D) {
			if (Ref<Vulkan::Image> image = descriptor->image.as<Vulkan::Image>(); descriptor->write_set.pImageInfo != nullptr
				&& image->get_descriptor_info().imageView != descriptor->write_set.pImageInfo->imageView) {
				pending_descriptors.emplace_back(descriptor);
				invalidate_descriptor_sets();
			}
		}
	}

	std::vector<VkDescriptorImageInfo> array_image_infos;

	dirty_descriptor_sets[frame_index.value] = false;
	write_descriptors[frame_index.value].clear();

	if (!uniform_write_descriptors.empty()) {
		for (const auto& write : uniform_write_descriptors[frame_index.value]) {
			write_descriptors[frame_index.value].push_back(write);
		}
	}

	for (const auto& pd : resident_descriptors | std::views::values) {
		if (pd->type == PendingDescriptorType::Texture2D) {
			auto texture = pd->texture.as<Vulkan::Texture>();
			pd->image_info = cast_to<Vulkan::Image>(texture->get_image(0)).get_descriptor_info();
			pd->write_set.pImageInfo = &pd->image_info;
		} else if (pd->type == PendingDescriptorType::TextureCube) {
			Ref<Vulkan::Texture3D> texture = pd->texture.as<Vulkan::Texture3D>();
			pd->image_info = cast_to<Vulkan::Image>(texture->get_image(0)).get_descriptor_info();
			pd->write_set.pImageInfo = &pd->image_info;
		} else if (pd->type == PendingDescriptorType::Image2D) {
			auto image = pd->image.as<Vulkan::Image>();
			pd->image_info = image->get_descriptor_info();
			pd->write_set.pImageInfo = &pd->image_info;
		}

		write_descriptors[frame_index.value].push_back(pd->write_set);
	}

	for (const auto& pd : resident_descriptor_arrays | std::views::values) {
		if (pd->type == PendingDescriptorType::Texture2D) {
			for (const auto& tex : pd->textures) {
				auto texture = tex.as<Vulkan::Texture>();
				array_image_infos.emplace_back(texture->get_descriptor_info());
			}
		}
		pd->write_set.pImageInfo = array_image_infos.data();
		pd->write_set.descriptorCount = static_cast<uint32_t>(array_image_infos.size());
		write_descriptors[frame_index.value].push_back(pd->write_set);
	}

	const auto descriptor_set = shader->allocate_descriptor_set();
	descriptor_sets[frame_index] = descriptor_set;
	for (auto& write_descriptor : write_descriptors[frame_index.value]) {
		write_descriptor.dstSet = descriptor_set.descriptor_sets[0];
	}

	const auto& write_descriptors_current_frame = write_descriptors[frame_index.value];
	vkUpdateDescriptorSets(
		vk_device, static_cast<std::uint32_t>(write_descriptors_current_frame.size()), write_descriptors_current_frame.data(), 0, nullptr);
	pending_descriptors.clear();
}

} // namespace Disarray::Vulkan
