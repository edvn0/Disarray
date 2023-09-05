#include "DisarrayPCH.hpp"

#include "graphics/PipelineCache.hpp"

#include <filesystem>

#include "core/Log.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/Shader.hpp"
#include "graphics/ShaderCompiler.hpp"
#include "graphics/Swapchain.hpp"

namespace Disarray {

PipelineCache::PipelineCache(const Disarray::Device& dev, const std::filesystem::path& base)
	: ResourceCache(dev, base, { ".vert", ".frag" })
{
	const auto all_files = get_unique_files_recursively();

	// create all pairs of shaders
	std::vector<std::filesystem::path> as_vector { all_files.begin(), all_files.end() };
	std::sort(as_vector.begin(), as_vector.end());

	Runtime::ShaderCompiler::initialize();
	Runtime::ShaderCompiler compiler {};

	for (const auto& shader_path : as_vector) {
		auto name = shader_path.filename();
		if (shader_cache.contains(name.string())) {
			break;
		}

		ShaderType type = to_shader_type(shader_path);
		auto code = compiler.compile(shader_path, type);

		auto shader = Shader::construct(get_device(),
			{
				.code = code,
				.identifier = shader_path,
				.type = type,
			});

		if (!shader) {
			DISARRAY_LOG_ERROR("Pipeline Cache - Shader Loading", "Tried to load shader {} but could not.", shader_path.string());
			continue;
		}

		auto filename = shader_path.filename().string();
		shader_cache.try_emplace(filename, std::move(shader));
	}
}

} // namespace Disarray
