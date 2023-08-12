#pragma once

#include "Forward.hpp"
#include "ResourceCache.hpp"
#include "core/Types.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PushConstantLayout.hpp"
#include "graphics/Shader.hpp"
#include "graphics/Swapchain.hpp"

#include <algorithm>
#include <filesystem>
#include <set>
#include <unordered_map>
#include <utility>

using VkDescriptorSetLayout = struct VkDescriptorSetLayout_T*;

namespace Disarray {

	class Framebuffer;

	struct PipelineCacheCreationProperties {
		std::string pipeline_key;
		std::string vertex_shader_key;
		std::string fragment_shader_key;
		Ref<Framebuffer> framebuffer { nullptr };
		Ref<RenderPass> render_pass { nullptr };
		VertexLayout layout;
		PushConstantLayout push_constant_layout;
		Extent extent { 0, 0 };
		PolygonMode polygon_mode { PolygonMode::Fill };
		float line_width { 1.0f };
		SampleCount samples { SampleCount::One };
		const VkDescriptorSetLayout* descriptor_set_layout { nullptr };
		std::uint32_t descriptor_set_layout_count { 0 };
	};

	class PipelineCache : public ResourceCache<Ref<Disarray::Pipeline>, PipelineCacheCreationProperties, PipelineCache, std::string, StringHash> {
	public:
		PipelineCache(Disarray::Device& device, const std::filesystem::path&);

		void force_recreate_impl(const Extent& extent)
		{
			for_each_in_storage([&extent](auto& resource) {
				auto& [k, v] = resource;
				v->recreate(true, extent);
			});
		}

		Ref<Disarray::Pipeline> create_from_impl(const PipelineCacheCreationProperties& props)
		{
			PipelineProperties properties {
				.vertex_shader = shader_cache[props.vertex_shader_key],
				.fragment_shader = shader_cache[props.fragment_shader_key],
				.framebuffer = props.framebuffer,
				.layout = props.layout,
				.push_constant_layout = props.push_constant_layout,
				.extent = props.extent,
				.polygon_mode = props.polygon_mode,
				.line_width = props.line_width,
				.samples = props.samples,
				.descriptor_set_layout = props.descriptor_set_layout,
				.descriptor_set_layout_count = props.descriptor_set_layout_count,
			};

			return Pipeline::construct(get_device(), properties);
		}

		std::string create_key_impl(const PipelineCacheCreationProperties& props) { return props.pipeline_key; }
		const Ref<Shader>& get_shader(const std::string&);

	private:
		std::unordered_map<std::string, Ref<Shader>> shader_cache {};
	};
} // namespace Disarray
