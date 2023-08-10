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

	PipelineCache::PipelineCache(Disarray::Device& dev, const std::filesystem::path& base)
		: ResourceCache(dev, base, { ".spv" })
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

				auto first_shader = Shader::construct(get_device(), { .path = current, .type = first_shader_type });
				auto second_shader = Shader::construct(get_device(), { .path = other, .type = second_shader_type });

				if (first_shader_type == ShaderType::Vertex && second_shader_type == ShaderType::Fragment) {
					shader_cache.try_emplace(name, std::make_pair(first_shader, second_shader));
				} else {
					shader_cache.try_emplace(name, std::make_pair(second_shader, first_shader));
				}
			}
		}
	}

	const PipelineCache::ShaderPair& PipelineCache::get_shader(const std::string& key) { return shader_cache[key]; }

} // namespace Disarray
