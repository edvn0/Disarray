#pragma once

#include <vulkan/vulkan.h>

#include <unordered_map>
#include <vector>

#include "core/DisarrayObject.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/RendererProperties.hpp"
#include "graphics/Texture.hpp"
#include "graphics/TextureCache.hpp"
#include "vulkan/UniformBuffer.hpp"

namespace Disarray::Vulkan {

class GraphicsResource : public IGraphicsResource {
	DISARRAY_MAKE_NONCOPYABLE(GraphicsResource)
public:
	GraphicsResource(const Disarray::Device&, const Disarray::Swapchain&);
	~GraphicsResource() override;

	void recreate(bool should_clean, const Extent& extent) override;

	[[nodiscard]] auto get_pipeline_cache() -> PipelineCache& override { return pipeline_cache; }
	[[nodiscard]] auto get_texture_cache() -> TextureCache& override { return texture_cache; }
	[[nodiscard]] auto get_pipeline_cache() const -> const PipelineCache& override { return pipeline_cache; }
	[[nodiscard]] auto get_texture_cache() const -> const TextureCache& override { return texture_cache; }

	void expose_to_shaders(const Disarray::UniformBuffer& uniform_buffer, DescriptorSet set, DescriptorBinding binding) override;
	void expose_to_shaders(std::span<const Ref<Disarray::Texture>> textures, DescriptorSet set, DescriptorBinding binding) override;
	void expose_to_shaders(std::span<const Disarray::Texture*> textures, DescriptorSet set, DescriptorBinding binding) override;
	void expose_to_shaders(const Disarray::StorageBuffer& buffer, DescriptorSet set, DescriptorBinding binding) override;
	void expose_to_shaders(const Disarray::Image& image, DescriptorSet set, DescriptorBinding binding) override;
	void expose_to_shaders(const Disarray::Texture& image, DescriptorSet set, DescriptorBinding binding) override
	{
		return expose_to_shaders(image.get_image(), set, binding);
	}
	[[nodiscard]] auto get_descriptor_set(FrameIndex frame_index, DescriptorSet set) const -> VkDescriptorSet override
	{
		return descriptor_sets.at(frame_index).at(set.value);
	}
	[[nodiscard]] auto get_descriptor_set(DescriptorSet set) const -> VkDescriptorSet override
	{
		return descriptor_sets.at(swapchain.get_current_frame_index()).at(set.value);
	}
	[[nodiscard]] auto get_descriptor_set() const -> VkDescriptorSet override
	{
		return get_descriptor_set(FrameIndex(swapchain.get_current_frame()), DescriptorSet(0));
	};
	[[nodiscard]] auto get_descriptor_set_layouts() const -> const std::vector<VkDescriptorSetLayout>& override { return layouts; }
	[[nodiscard]] auto get_push_constant() const -> const PushConstant* override { return &pc; }
	auto get_editable_push_constant() -> PushConstant& override { return pc; }

	void push_constant(Disarray::CommandExecutor&, const Disarray::Pipeline&) override;
	void push_constant(Disarray::CommandExecutor&, const Disarray::Pipeline&, const void* data, std::size_t size) override;

private:
	void cleanup_graphics_resource();
	auto descriptor_write_sets_per_frame(DescriptorSet descriptor_set) -> std::vector<VkWriteDescriptorSet>;

	const Disarray::Device& device;
	const Disarray::Swapchain& swapchain;
	std::uint32_t swapchain_image_count { 0 };

	Disarray::PipelineCache pipeline_cache;
	Disarray::TextureCache texture_cache;

	VkDescriptorPool pool { nullptr };
	PushConstant pc {};

	std::unordered_map<FrameIndex, std::vector<VkDescriptorSet>> descriptor_sets;
	std::vector<VkDescriptorSetLayout> layouts;
	void initialise_descriptors(bool should_clean = false);

	void internal_expose_to_shaders(
		VkSampler sampler, const std::vector<VkDescriptorImageInfo>& image_infos, DescriptorSet set, DescriptorBinding binding);
};

} // namespace Disarray::Vulkan
