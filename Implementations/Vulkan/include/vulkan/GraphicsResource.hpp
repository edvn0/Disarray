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

// TODO: Make this dynamic
static constexpr auto set_count = 3;

class GraphicsResource : public IGraphicsResource {
	DISARRAY_MAKE_NONCOPYABLE(GraphicsResource)
public:
	GraphicsResource(const Disarray::Device&, const Disarray::Swapchain&);
	~GraphicsResource() override;

	[[nodiscard]] auto get_pipeline_cache() -> PipelineCache& override { return pipeline_cache; }
	[[nodiscard]] auto get_texture_cache() -> TextureCache& override { return texture_cache; }

	void expose_to_shaders(std::span<const Ref<Disarray::Texture>>) override;
	void expose_to_shaders(const Disarray::Image&) override;
	void expose_to_shaders(const Disarray::Texture& tex) override { expose_to_shaders(tex.get_image()); };
	[[nodiscard]] auto get_descriptor_set(std::uint32_t frame_index, std::uint32_t set) const -> VkDescriptorSet override
	{
		return descriptor_sets.at(frame_index).at(set);
	}
	[[nodiscard]] auto get_descriptor_set() const -> VkDescriptorSet override { return get_descriptor_set(swapchain.get_current_frame(), 0); };
	[[nodiscard]] auto get_descriptor_set_layouts() const -> const std::vector<VkDescriptorSetLayout>& override { return layouts; }
	[[nodiscard]] auto get_push_constant() const -> const PushConstant* override { return &pc; }
	auto get_editable_push_constant() -> PushConstant& override { return pc; }

	auto get_editable_ubos() -> std::tuple<UBO&, CameraUBO&, PointLights&, ImageIndicesUBO&> override
	{
		return { uniform, camera_ubo, lights, image_indices };
	}

	void update_ubo() override;

private:
	const Disarray::Device& device;
	const Disarray::Swapchain& swapchain;

	Disarray::PipelineCache pipeline_cache;
	Disarray::TextureCache texture_cache;

	VkDescriptorPool pool { nullptr };
	PushConstant pc {};

	UBO uniform {};
	CameraUBO camera_ubo {};
	PointLights lights {};
	ImageIndicesUBO image_indices {};

	using UBOArray = std::array<Scope<Vulkan::UniformBuffer>, 4>;
	std::unordered_map<std::size_t, UBOArray> frame_index_ubo_map {};

	std::unordered_map<std::size_t, std::vector<VkDescriptorSet>> descriptor_sets;
	std::vector<VkDescriptorSetLayout> layouts;
	void initialise_descriptors();
};

} // namespace Disarray::Vulkan
