#include "DisarrayPCH.hpp"

#include "graphics/PipelineCache.hpp"

#include "core/Log.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/Shader.hpp"
#include "graphics/Swapchain.hpp"

#include <filesystem>

static constexpr auto trim_extensions = [](const auto& p) {
	if (p.extension() == ".spv")
		return p.filename().replace_extension().replace_extension();
	else
		return p.filename().replace_extension();
};

static constexpr auto extract_name = [](const auto& p) {
	if (p.extension() == ".spv")
		return p.filename().replace_extension().replace_extension().string();
	else
		return p.filename().replace_extension().string();
};

static constexpr auto is_vertex = [](const auto& p) {
	if (p.extension() == ".spv")
		return p.filename().replace_extension().extension() == ".vert";
	else
		return p.filename().extension() == ".vert";
};

static constexpr auto compare_on_filename = [](const std::filesystem::path& left) {
	// x.{vert,frag}.spv => x
	const auto file_name = trim_extensions(left);
	return [file_name](const std::filesystem::path& right) {
		const auto other_file_name = right.filename();

		auto trimmed = trim_extensions(other_file_name);

		return trimmed == file_name;
	};
};

namespace Disarray {

	PipelineCache::~PipelineCache()
	{
		shader_cache.clear();
		pipeline_cache.clear();
	}

	PipelineCache::PipelineCache(Disarray::Device& dev, Disarray::Swapchain& sc, const std::filesystem::path& base)
		: device(dev)
		, swapchain(sc)
		, path(base)
	{
		const auto all_files = get_unique_files_recursively();

		// create all pairs of shaders
		std::vector<std::filesystem::path> as_vector { all_files.begin(), all_files.end() };
		std::ranges::sort(as_vector.begin(), as_vector.end());

		for (std::size_t i = 0; i < as_vector.size(); i++) {
			const auto& current = as_vector[i];
			const auto compare_to_other = compare_on_filename(current);
			for (std::size_t j = i + 1; j < as_vector.size(); j++) {
				const auto& other = as_vector[j];
				if (!compare_to_other(other))
					continue;

				auto name = extract_name(current);
				if (shader_cache.contains(name))
					break;

				auto first_shader_type = is_vertex(current) ? ShaderType::Vertex : ShaderType::Fragment;
				auto second_shader_type = is_vertex(other) ? ShaderType::Vertex : ShaderType::Fragment;

				auto first_shader = Shader::construct(device, { .path = current, .type = first_shader_type });
				auto second_shader = Shader::construct(device, { .path = other, .type = second_shader_type });

				if (first_shader_type == ShaderType::Vertex && second_shader_type == ShaderType::Fragment) {
					shader_cache.try_emplace(name, std::make_pair(first_shader, second_shader));
				} else {
					shader_cache.try_emplace(name, std::make_pair(second_shader, first_shader));
				}
			}
		}
	}

	std::set<std::filesystem::path> PipelineCache::get_unique_files_recursively() const
	{
		std::set<std::filesystem::path> paths;
		for (const auto& current : std::filesystem::recursive_directory_iterator { path }) {
			if (!current.is_regular_file() || current.path().extension() != ".spv")
				continue;

			paths.insert(current);
		}
		return paths;
	}

	void PipelineCache::force_recreation()
	{
		CollectionOperations::for_each(pipeline_cache, [](PipelineCacheValueType& pipeline) { pipeline.second->force_recreation(); });
	}

	const Ref<Disarray::Pipeline>& PipelineCache::get(const std::string& key) { return pipeline_cache[key]; }

	const Ref<Disarray::Pipeline>& PipelineCache::put(const PipelineCacheCreationProperties& props)
	{
		const auto& [vert, frag] = shader_cache[props.shader_key];
		PipelineProperties properties {
			.vertex_shader = vert,
			.fragment_shader = frag,
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

		auto pipeline = Pipeline::construct(device, properties);
		const auto& [pair, could] = pipeline_cache.try_emplace(props.pipeline_key, std::move(pipeline));
		if (!could)
			Log::error("PipelineCache - Put", "Could not insert pipeline.");
		return pair->second;
	}

	const PipelineCache::ShaderPair& PipelineCache::get_shader(const std::string& key) { return shader_cache[key]; }

} // namespace Disarray
