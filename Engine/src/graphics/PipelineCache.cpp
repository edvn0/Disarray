#include "graphics/PipelineCache.hpp"

#include "graphics/Framebuffer.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/Shader.hpp"
#include "graphics/Swapchain.hpp"

#include <filesystem>

static constexpr auto extract_shader_type = [](const auto& p) {
	if (p.extension() == ".spv")
		return p.filename().replace_extension().string();
	else
		return p.filename().string();
};
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
		std::sort(as_vector.begin(), as_vector.end());

		for (auto i = 0; i < as_vector.size(); i++) {
			auto current = as_vector[i];
			const auto compare_to_other = compare_on_filename(current);
			for (auto j = i + 1; j < as_vector.size(); j++) {
				auto other = as_vector[j];
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
					shader_cache.insert(std::make_pair(name, std::make_pair(first_shader, second_shader)));
				} else {
					shader_cache.insert(std::make_pair(name, std::make_pair(second_shader, first_shader)));
				}

				break;
			}
		}
	}

	std::set<std::filesystem::path> PipelineCache::get_unique_files_recursively()
	{
		std::set<std::filesystem::path> paths;
		for (const auto current : std::filesystem::recursive_directory_iterator { path }) {
			if (!current.is_regular_file() || current.path().extension() != ".spv")
				continue;

			paths.insert(current);
		}
		return paths;
	}

	void PipelineCache::force_recreation()
	{
		for_each(pipeline_cache, [](PipelineCacheValueType& pipeline) { pipeline.second->force_recreation(); });
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
		};

		auto pipeline = Pipeline::construct(device, swapchain, properties);
		auto out = pipeline_cache.insert(std::make_pair(props.pipeline_key, std::move(pipeline)));
		return out.first->second;
	}

	const PipelineCache::ShaderPair& PipelineCache::get_shader(const std::string& key) { return shader_cache[key]; }

} // namespace Disarray
