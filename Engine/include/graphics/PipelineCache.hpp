#pragma once

#include "Forward.hpp"
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

	namespace CollectionOperations {

		template <typename Collection, typename Func> static inline void for_each(Collection& coll, Func&& func)
		{
			std::ranges::for_each(std::begin(coll), std::end(coll), std::forward<Func>(func));
		}

	} // namespace CollectionOperations

	class Framebuffer;

	struct PipelineCacheCreationProperties {
		std::string pipeline_key;
		std::string shader_key;
		Ref<Framebuffer> framebuffer { nullptr };
		Ref<RenderPass> render_pass { nullptr };
		VertexLayout layout;
		PushConstantLayout push_constant_layout;
		Extent extent { 0, 0 };
		PolygonMode polygon_mode { PolygonMode::Fill };
		float line_width { 1.0f };
		SampleCount samples { SampleCount::ONE };
		const VkDescriptorSetLayout* descriptor_set_layout { nullptr };
		std::uint32_t descriptor_set_layout_count { 0 };
	};

	class PipelineCache {
		struct StringHash {
			using is_transparent = void; // enables heterogeneous lookup

			std::size_t operator()(std::string_view sv) const
			{
				std::hash<std::string_view> hasher;
				return hasher(sv);
			}
		};

		using ShaderPair = std::pair<Ref<Disarray::Shader>, Ref<Disarray::Shader>>;
		using PipelineMap = std::unordered_map<std::string, Ref<Disarray::Pipeline>, StringHash, std::equal_to<>>;
		using PipelineCacheValueType = PipelineMap::value_type;

	public:
		PipelineCache(Disarray::Device& device, Disarray::Swapchain&, const std::filesystem::path&);
		~PipelineCache();

		const Ref<Disarray::Pipeline>& get(const std::string&);
		const Ref<Disarray::Pipeline>& put(const PipelineCacheCreationProperties&);

		const ShaderPair& get_shader(const std::string&);

		void force_recreation();

	private:
		std::set<std::filesystem::path> get_unique_files_recursively() const;

		Disarray::Device& device;
		Disarray::Swapchain& swapchain;
		std::filesystem::path path { "Assets/Shaders" };

		PipelineMap pipeline_cache {};
		std::unordered_map<std::string, ShaderPair> shader_cache {};
	};
} // namespace Disarray
