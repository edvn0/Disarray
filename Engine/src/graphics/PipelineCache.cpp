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
	: ResourceCache(dev, base, { ".spv" })
{
	const auto all_files = get_unique_files_recursively();

	// create all pairs of shaders
	std::vector<std::filesystem::path> as_vector { all_files.begin(), all_files.end() };
	std::sort(as_vector.begin(), as_vector.end());

	Runtime::ShaderCompiler::initialize();
	Runtime::ShaderCompiler compiler {};

	for (const auto& shader_path : as_vector) {
		auto name = shader_path.filename();
		const auto filename_without_spv = name.replace_extension().string();
		if (shader_cache.contains(name.string())) {
			break;
		}

		auto path_copy = shader_path;
		const auto& shader_extension_without_spv = path_copy.replace_extension();

		ShaderType type = to_shader_type(shader_extension_without_spv);
		compiler.compile(shader_extension_without_spv, type);

		auto shader = Shader::construct(get_device(),
			{
				.path = shader_path,
				.type = type,
			});

		if (!shader) {
			Log::error("Pipeline Cache - Shader Loading", "Tried to load shader {} but could not.", shader_path.string());
			continue;
		}

		shader_cache.try_emplace(filename_without_spv, std::move(shader));
	}

	Runtime::ShaderCompiler::destroy();
}

} // namespace Disarray
