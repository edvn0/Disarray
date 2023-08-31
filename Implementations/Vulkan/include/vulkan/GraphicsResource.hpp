#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "core/DisarrayObject.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Texture.hpp"
#include "graphics/TextureCache.hpp"
#include "vulkan/UniformBuffer.hpp"

namespace Disarray::Vulkan {

// TODO: Make this dynamic
static constexpr auto set_count = 2;

class GraphicsResource : public IGraphicsResource {
	DISARRAY_MAKE_NONCOPYABLE(GraphicsResource)
public:
	GraphicsResource(const Disarray::Device&, const Disarray::Swapchain&);
	~GraphicsResource() override;

	[[nodiscard]] auto get_pipeline_cache() -> PipelineCache& override { return pipeline_cache; }
	[[nodiscard]] auto get_texture_cache() -> TextureCache& override { return texture_cache; }

	void expose_to_shaders(Disarray::Image&) override;
	void expose_to_shaders(Disarray::Texture& tex) override { expose_to_shaders(tex.get_image()); };
	[[nodiscard]] auto get_descriptor_set(std::uint32_t frame_index, std::uint32_t set) const -> VkDescriptorSet override
	{
		return descriptor_sets[(frame_index * set_count) + set];
	}
	[[nodiscard]] auto get_descriptor_set() const -> VkDescriptorSet override { return get_descriptor_set(swapchain.get_current_frame(), 0); };
	[[nodiscard]] auto get_descriptor_set_layouts() const -> const std::vector<VkDescriptorSetLayout>& override { return layouts; }
	[[nodiscard]] auto get_push_constant() const -> const PushConstant* override { return &pc; }
	auto get_editable_push_constant() -> PushConstant& override { return pc; }

	auto get_editable_ubos() -> std::tuple<UBO&, CameraUBO&, PointLights&> override { return { uniform, camera_ubo, lights }; }

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
	std::vector<std::array<Scope<Vulkan::UniformBuffer>, 3>> frame_ubos;

	std::vector<VkDescriptorSet> descriptor_sets;
	std::vector<VkDescriptorSetLayout> layouts;
	void initialise_descriptors();
};

} // namespace Disarray::Vulkan
