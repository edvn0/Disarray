#pragma once

#include "Forward.hpp"
#include "core/Types.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PushContantLayout.hpp"
#include "graphics/Swapchain.hpp"

#include <algorithm>
#include <filesystem>
#include <set>
#include <unordered_map>
#include <utility>

namespace Disarray {

	struct PipelineCacheCreationProperties {
		std::string pipeline_key;
		std::string shader_key;
		Ref<Framebuffer> framebuffer {nullptr};
		Ref<RenderPass> render_pass {nullptr};
		VertexLayout layout;
		PushConstantLayout push_constant_layout;
		Extent extent {0,0};
		PolygonMode polygon_mode { PolygonMode::Fill };
		float line_width {1.0f};
	};

	class PipelineCache {
		using ShaderPair = std::pair<Ref<Disarray::Shader>, Ref<Disarray::Shader>>;
		using PipelineMap = std::unordered_map<std::string, Ref<Disarray::Pipeline>>;
		using PipelineCacheValueType = PipelineMap::value_type;

	public:
		PipelineCache(Disarray::Device& device, Disarray::Swapchain&, const std::filesystem::path&);
		~PipelineCache();

		const Ref<Disarray::Pipeline>& get(const std::string&);
		const Ref<Disarray::Pipeline>& put(const PipelineCacheCreationProperties&);

		const ShaderPair& get_shader(const std::string&);

		void force_recreation();

	private:
		std::set<std::filesystem::path> get_unique_files_recursively();

		template<typename Collection, typename Func>
		static inline void for_each(Collection& coll, Func&& func) {
			std::ranges::for_each(coll.begin(), coll.end(), func);
		}
		Disarray::Device& device;
		Disarray::Swapchain& swapchain;
		std::filesystem::path path {"Assets/Shaders"};

		PipelineMap pipeline_cache {};
		std::unordered_map<std::string, ShaderPair> shader_cache {};
	};
}