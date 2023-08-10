#include "DisarrayPCH.hpp"

#include "graphics/PipelineCache.hpp"

#include "core/Log.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/Shader.hpp"
#include "graphics/Swapchain.hpp"

#include <filesystem>

namespace Disarray {

	PipelineCache::PipelineCache(Disarray::Device& dev, const std::filesystem::path& base)
		: ResourceCache(dev, base, { ".spv" })
	{
		const auto all_files = get_unique_files_recursively();

		// create all pairs of shaders
		std::vector<std::filesystem::path> as_vector { all_files.begin(), all_files.end() };
		std::ranges::sort(as_vector.begin(), as_vector.end());

		for (const auto& shader_path : as_vector) {
			auto name = shader_path.filename();
			if (shader_cache.contains(name.string()))
				break;

			auto path_copy = shader_path;
			const auto& shader_extension_without_spv = path_copy.replace_extension();
			ShaderType type = ShaderType::Vertex;
			if (shader_extension_without_spv.extension() == ".frag")
				type = ShaderType::Fragment;
			if (shader_extension_without_spv.extension() == ".comp")
				type = ShaderType::Compute;

			auto shader = Shader::construct(get_device(),
				{
					.path = shader_path,
					.type = type,
				});

			if (!shader) {
				Log::error("Pipeline Cache - Shader Loading", "Tried to load shader {} but could not.", shader_path.string());
				continue;
			}

			const auto filename_without_spv = name.replace_extension();
			shader_cache.try_emplace(filename_without_spv.string(), std::move(shader));
		}
	}

	const Ref<Shader>& PipelineCache::get_shader(const std::string& key) { return shader_cache[key]; }

} // namespace Disarray
