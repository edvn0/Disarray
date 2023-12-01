#include "DisarrayPCH.hpp"

#include <vulkan/vulkan.h>

#include <ranges>

#include "core/Types.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/Renderer.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/GraphicsResource.hpp"
#include "vulkan/Image.hpp"
#include "vulkan/Material.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/Renderer.hpp"
#include "vulkan/Texture.hpp"
#include "vulkan/UnifiedShader.hpp"

namespace Disarray::Vulkan {

namespace {
	auto create_set_zero_bindings()
	{
		auto default_binding = vk_structures<VkDescriptorSetLayoutBinding> {}();
		default_binding.descriptorCount = 1;
		default_binding.binding = 0;
		default_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		default_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

		auto camera_binding = vk_structures<VkDescriptorSetLayoutBinding> {}();
		camera_binding.descriptorCount = 1;
		camera_binding.binding = 1;
		camera_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		camera_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

		auto point_light_binding = vk_structures<VkDescriptorSetLayoutBinding> {}();
		point_light_binding.descriptorCount = 1;
		point_light_binding.binding = 2;
		point_light_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		point_light_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

		auto shadow_pass_binding = vk_structures<VkDescriptorSetLayoutBinding> {}();
		shadow_pass_binding.descriptorCount = 1;
		shadow_pass_binding.binding = 3;
		shadow_pass_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		shadow_pass_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

		auto directional_light_binding = vk_structures<VkDescriptorSetLayoutBinding> {}();
		directional_light_binding.descriptorCount = 1;
		directional_light_binding.binding = 4;
		directional_light_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		directional_light_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

		auto glyph_binding = vk_structures<VkDescriptorSetLayoutBinding> {}();
		glyph_binding.descriptorCount = 1;
		glyph_binding.binding = 5;
		glyph_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		glyph_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

		auto spot_light_binding = vk_structures<VkDescriptorSetLayoutBinding> {}();
		spot_light_binding.descriptorCount = 1;
		spot_light_binding.binding = 6;
		spot_light_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		spot_light_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

		return std::array {
			default_binding,
			camera_binding,
			point_light_binding,
			shadow_pass_binding,
			directional_light_binding,
			glyph_binding,
			spot_light_binding,
		};
	}

	auto create_set_one_bindings()
	{
		// Default framebuffer
		auto image_binding_0 = vk_structures<VkDescriptorSetLayoutBinding> {}();
		image_binding_0.descriptorCount = 1;
		image_binding_0.binding = 0;
		image_binding_0.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		image_binding_0.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// Depth texture
		auto image_binding_1 = vk_structures<VkDescriptorSetLayoutBinding> {}();
		image_binding_1.descriptorCount = 1;
		image_binding_1.binding = 1;
		image_binding_1.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		image_binding_1.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// Font texture
		auto image_binding_2 = vk_structures<VkDescriptorSetLayoutBinding> {}();
		image_binding_2.descriptorCount = 1;
		image_binding_2.binding = 2;
		image_binding_2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		image_binding_2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		return std::array {
			image_binding_0,
			image_binding_1,
			image_binding_2,
		};
	}

} // namespace

Material::Material(const Disarray::Device& dev, const Disarray::MaterialProperties& properties)
	: Disarray::Material(properties)
	, device(dev)
{
	recreate_material(false);
};

Material::~Material() = default;

void Material::update_material(Disarray::Renderer& renderer)
{
	if (!needs_update) {
		return;
	}

	auto& graphics_resource = cast_to<Vulkan::GraphicsResource>(renderer.get_graphics_resource());
	recreate_descriptor_set_layout(graphics_resource);

	needs_update = false;
}

void Material::write_textures(IGraphicsResource& resource) const
{
	for (auto i = FrameIndex { 0 }; i < resource.get_image_count(); i++) {
		auto* set = resource.get_descriptor_set(i, DescriptorSet(0));

		std::vector<VkWriteDescriptorSet> write_descriptors {};
		std::uint32_t binding = 17;
		Collections::for_each_unwrapped(props.textures, [&](const auto&, const auto& texture) mutable {
			const auto& vk_image = cast_to<Vulkan::Image>(texture->get_image());
			auto& write_descriptor_set = write_descriptors.emplace_back();
			write_descriptor_set.descriptorCount = 1;
			write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write_descriptor_set.dstSet = set;
			write_descriptor_set.dstBinding = binding++;
			write_descriptor_set.pImageInfo = &vk_image.get_descriptor_info();
			write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		});

		vkUpdateDescriptorSets(
			supply_cast<Vulkan::Device>(device), static_cast<std::uint32_t>(write_descriptors.size()), write_descriptors.data(), 0, nullptr);
	}
}

void Material::bind(const Disarray::CommandExecutor& executor, const Disarray::Pipeline& pipeline, FrameIndex index) const
{
	auto* command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);
	auto* pipeline_layout = cast_to<Vulkan::Pipeline>(pipeline).get_layout();

	// Always bound to set 2 bindings 3,4,5
	auto descriptor_set = frame_based_descriptor_sets.at(index);

	std::array descriptor_sets {
		descriptor_set.at(0),
	};
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, static_cast<std::uint32_t>(descriptor_sets.size()),
		descriptor_sets.data(), 0, nullptr);
}

void Material::recreate_material(bool should_clean) { }

void Material::recreate_descriptor_set_layout(Vulkan::GraphicsResource& resource)
{
	std::vector<VkDescriptorSetLayoutBinding> bindings {};
	Collections::for_each<true>(props.textures, [&binds = bindings](auto&, std::size_t i) {
		auto& binding = binds.emplace_back();

		binding.binding = static_cast<std::uint32_t>(i) + 3;
		binding.descriptorCount = 1;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	});

	VkDescriptorSetLayoutCreateInfo create_info {};
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	create_info.bindingCount = static_cast<std::uint32_t>(bindings.size());
	create_info.pBindings = bindings.data();

	VkDescriptorSetLayout set_two_layout;
	verify(vkCreateDescriptorSetLayout(supply_cast<Vulkan::Device>(device), &create_info, nullptr, &set_two_layout));

	auto set_zero_bindings = create_set_zero_bindings();
	VkDescriptorSetLayout set_zero_layout;
	create_info.pBindings = set_zero_bindings.data();
	create_info.bindingCount = static_cast<std::uint32_t>(set_zero_bindings.size());
	verify(vkCreateDescriptorSetLayout(supply_cast<Vulkan::Device>(device), &create_info, nullptr, &set_zero_layout));

	auto set_one_bindings = create_set_one_bindings();
	VkDescriptorSetLayout set_one_layout;
	create_info.pBindings = set_one_bindings.data();
	create_info.bindingCount = static_cast<std::uint32_t>(set_one_bindings.size());
	verify(vkCreateDescriptorSetLayout(supply_cast<Vulkan::Device>(device), &create_info, nullptr, &set_one_layout));

	layouts = { set_zero_layout, set_one_layout, set_two_layout };

	VkDescriptorSetAllocateInfo alloc_info {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorSetCount = static_cast<std::uint32_t>(layouts.size());
	alloc_info.pSetLayouts = layouts.data();

	for (auto i = FrameIndex { 0 }; i < resource.get_image_count(); i++) {
		std::vector<VkDescriptorSet> desc_sets {};
		desc_sets.resize(layouts.size());
		resource.allocate_descriptor_sets(alloc_info, desc_sets);
		frame_based_descriptor_sets.try_emplace(i, std::move(desc_sets));
	}

	for (auto i = FrameIndex { 0 }; i < resource.get_image_count(); i++) {
		auto* set = frame_based_descriptor_sets.at(i).at(2);

		std::vector<VkWriteDescriptorSet> write_descriptors {};
		std::uint32_t binding = 3;
		Collections::for_each_unwrapped(props.textures, [&](const auto&, const auto& texture) mutable {
			const auto& vk_image = cast_to<Vulkan::Image>(texture->get_image());
			auto& write_descriptor_set = write_descriptors.emplace_back();
			write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_descriptor_set.descriptorCount = 1;
			write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write_descriptor_set.dstSet = set;
			write_descriptor_set.dstBinding = binding++;
			write_descriptor_set.pImageInfo = &vk_image.get_descriptor_info();
		});

		vkUpdateDescriptorSets(
			supply_cast<Vulkan::Device>(device), static_cast<std::uint32_t>(write_descriptors.size()), write_descriptors.data(), 0, nullptr);
	}
}

void Material::recreate(bool should_clean, const Extent&) { recreate_material(should_clean); }

void Material::force_recreation() { recreate_material(true); }

POCMaterial::POCMaterial(const Disarray::Device& dev, POCMaterialProperties properties)
	: Disarray::POCMaterial(std::move(properties))
	, write_descriptors { props.swapchain_image_count }
	, dirty_descriptor_sets(props.swapchain_image_count, false)
	, device(dev)
{
	recreate_material(false, {});
}

auto POCMaterial::invalidate() -> void
{
	auto shader = props.shader.as<Vulkan::UnifiedShader>();
	if (shader->has_descriptor_set(0)) {
		const auto& shader_descriptor_sets = shader->get_shader_descriptor_sets();
		if (!shader_descriptor_sets.empty()) {
			for (auto&& [binding, descriptor] : resident_descriptors) {
				pending_descriptors.push_back(descriptor);
			}
		}
	}
}

auto POCMaterial::recreate_material(bool should_clean, const Extent&) -> void
{
	if (should_clean) {
		clean_material();
	}

	allocate_buffer_storage();
	invalidate();
}

auto POCMaterial::find_uniform_declaration(const std::string& name) -> const Reflection::ShaderUniform*
{
	const auto& shader_buffers = cast_to<Vulkan::UnifiedShader>(*props.shader).get_shader_buffers();
	if (shader_buffers.empty()) {
		return nullptr;
	}

	const auto& [Name, Size, Uniforms] = shader_buffers.begin()->second;
	if (!Uniforms.contains(name)) {
		return nullptr;
	}

	return &Uniforms.at(name);
}

auto POCMaterial::find_resource_declaration(const std::string& name) -> const Reflection::ShaderResourceDeclaration*
{
	if (const auto& resources = cast_to<Vulkan::UnifiedShader>(*props.shader).get_resources(); resources.contains(name)) {
		return &resources.at(name);
	}

	return nullptr;
}

auto POCMaterial::clean_material() -> void { }

auto POCMaterial::allocate_buffer_storage() -> void
{

	if (const auto& shader_buffers = cast_to<Vulkan::UnifiedShader>(*props.shader).get_shader_buffers(); !shader_buffers.empty()) {
		std::uint32_t size = 0;
		for (auto&& [id, buffer_size, buffer] : std::ranges::views::values(shader_buffers)) {
			size += buffer_size;
		}
		uniform_storage_buffer.allocate(size);
		uniform_storage_buffer.zero_initialise();
	}
}

void POCMaterial::set(const std::string& name, float value) { set<>(name, value); }
void POCMaterial::set(const std::string& name, int value) { set<>(name, value); }
void POCMaterial::set(const std::string& name, std::uint32_t value) { set<>(name, value); }
void POCMaterial::set(const std::string& name, bool value) { set<>(name, value); }
void POCMaterial::set(const std::string& name, const glm::ivec2& value) { set<>(name, value); }
void POCMaterial::set(const std::string& name, const glm::ivec3& value) { set<>(name, value); }
void POCMaterial::set(const std::string& name, const glm::ivec4& value) { set<>(name, value); }
void POCMaterial::set(const std::string& name, const glm::uvec2& value) { set<>(name, value); }
void POCMaterial::set(const std::string& name, const glm::uvec3& value) { set<>(name, value); }
void POCMaterial::set(const std::string& name, const glm::uvec4& value) { set<>(name, value); }
void POCMaterial::set(const std::string& name, const glm::vec2& value) { set<>(name, value); }
void POCMaterial::set(const std::string& name, const glm::vec3& value) { set<>(name, value); }
void POCMaterial::set(const std::string& name, const glm::vec4& value) { set<>(name, value); }
void POCMaterial::set(const std::string& name, const glm::mat3& value) { set<>(name, value); }
void POCMaterial::set(const std::string& name, const glm::mat4& value) { set<>(name, value); }

auto POCMaterial::invalidate_descriptor_sets() -> void
{
	for (auto i = FrameIndex { 0 }; i < props.swapchain_image_count; i++) {
		dirty_descriptor_sets.at(i.value) = true;
	}
}

void POCMaterial::set_vulkan_descriptor(const std::string& name, const Ref<Vulkan::Texture>& texture)
{
	const auto* resource = find_resource_declaration(name);
	const std::uint32_t binding = resource->get_register();

	if (binding < textures.size() && textures[binding] && texture->hash() == textures[binding]->hash() && resident_descriptors.contains(binding)) {
		return;
	}

	if (binding >= textures.size()) {
		textures.resize(binding + 1);
	}
	textures[binding] = texture;

	const auto* wds = props.shader.as<Vulkan::UnifiedShader>()->get_descriptor_set(name);
	resident_descriptors[binding]
		= std::make_shared<PendingDescriptor>(PendingDescriptor { PendingDescriptorType::Texture2D, *wds, {}, texture, nullptr });
	pending_descriptors.push_back(resident_descriptors.at(binding));

	invalidate_descriptor_sets();
}

auto POCMaterial::set_vulkan_descriptor(const std::string& name, const Ref<Vulkan::Texture3D>& texture) -> void
{
	const auto* resource = find_resource_declaration(name);

	const std::uint32_t binding = resource->get_register();
	// Texture is already set
	if (binding < textures.size() && textures[binding] && texture->hash() == textures[binding]->hash()
		&& resident_descriptors.find(binding) != resident_descriptors.end()) {
		return;
	}

	if (binding >= textures.size()) {
		textures.resize(binding + 1);
	}
	textures[binding] = texture;

	const auto* wds = props.shader.as<Vulkan::UnifiedShader>()->get_descriptor_set(name);
	resident_descriptors[binding]
		= std::make_shared<PendingDescriptor>(PendingDescriptor { PendingDescriptorType::TextureCube, *wds, {}, texture, nullptr });
	pending_descriptors.push_back(resident_descriptors.at(binding));

	invalidate_descriptor_sets();
}

void POCMaterial::set_vulkan_descriptor(const std::string& name, const Ref<Vulkan::Texture>& texture, uint32_t array_index)
{
	const auto* resource = find_resource_declaration(name);

	const std::uint32_t binding = resource->get_register();
	// Texture is already set
	if (binding < texture_arrays.size() && texture_arrays[binding].size() < array_index
		&& texture->hash() == texture_arrays[binding][array_index]->hash()) {
		return;
	}

	if (binding >= texture_arrays.size()) {
		texture_arrays.resize(binding + 1);
	}

	if (array_index >= texture_arrays[binding].size()) {
		texture_arrays[binding].resize(array_index + 1);
	}

	texture_arrays[binding][array_index] = texture;

	const auto* wds = props.shader.as<Vulkan::UnifiedShader>()->get_descriptor_set(name);
	if (!resident_descriptor_arrays.contains(binding)) {
		resident_descriptor_arrays[binding]
			= std::make_shared<PendingDescriptorArray>(PendingDescriptorArray { PendingDescriptorType::Texture2D, *wds, {}, {}, {} });
	}

	const auto& resident_descriptor_array = resident_descriptor_arrays.at(binding);
	if (array_index >= resident_descriptor_array->textures.size()) {
		resident_descriptor_array->textures.resize(array_index + 1);
	}

	resident_descriptor_array->textures[array_index] = texture;

	invalidate_descriptor_sets();
}

void POCMaterial::set_vulkan_descriptor(const std::string& name, const Ref<Vulkan::Image>& image)
{
	const auto* resource = find_resource_declaration(name);

	const std::uint32_t binding = resource->get_register();
	if (binding < images.size() && images[binding] && resident_descriptors.contains(binding)) {
		return;
	}

	if (resource->get_register() >= images.size()) {
		images.resize(resource->get_register() + 1);
	}
	images[resource->get_register()] = image;

	const auto* wds = props.shader.as<Vulkan::UnifiedShader>()->get_descriptor_set(name);
	resident_descriptors[binding]
		= std::make_shared<PendingDescriptor>(PendingDescriptor { PendingDescriptorType::Image2D, *wds, {}, nullptr, image });
	pending_descriptors.push_back(resident_descriptors.at(binding));

	invalidate_descriptor_sets();
}

void POCMaterial::set(const std::string& name, const Ref<Disarray::Texture>& value)
{
	if (value->get_properties().dimension == Disarray::TextureDimension::Two) {
		set_vulkan_descriptor(name, value.as<Vulkan::Texture>());
	} else {
		set_vulkan_descriptor(name, value.as<Vulkan::Texture3D>());
	}
}

void POCMaterial::set(const std::string& name, const Ref<Disarray::Texture>& texture, std::uint32_t value)
{
	set_vulkan_descriptor(name, texture.as<Vulkan::Texture>(), value);
}

void POCMaterial::set(const std::string& name, const Ref<Disarray::Image>& image) { set_vulkan_descriptor(name, image.as<Vulkan::Image>()); }

auto POCMaterial::update_for_rendering(FrameIndex frame_index, const std::vector<std::vector<VkWriteDescriptorSet>>& uniform_write_descriptors)
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

	for (auto&& [binding, pd] : resident_descriptors) {
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

	for (auto&& [binding, pd] : resident_descriptor_arrays) {
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

	auto shader = props.shader.as<Vulkan::UnifiedShader>();
	const auto descriptor_set = shader->allocate_descriptor_set();
	descriptor_sets[frame_index] = descriptor_set;
	for (auto& write_descriptor : write_descriptors[frame_index.value]) {
		write_descriptor.dstSet = descriptor_set.descriptor_sets[0];
	}

	vkUpdateDescriptorSets(
		vk_device, static_cast<std::uint32_t>(write_descriptors[frame_index.value].size()), write_descriptors[frame_index.value].data(), 0, nullptr);
	pending_descriptors.clear();
}

} // namespace Disarray::Vulkan
