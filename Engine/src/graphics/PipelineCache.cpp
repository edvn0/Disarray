#include "DisarrayPCH.hpp"

#include <filesystem>
#include <ranges>
#include <vector>

#include "core/Log.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Shader.hpp"
#include "graphics/ShaderCompiler.hpp"
#include "graphics/Swapchain.hpp"

namespace Disarray {

PipelineCache::PipelineCache(const Disarray::Device& dev, const std::filesystem::path& base)
	: ResourceCache(dev, base, { ".vert", ".frag" })
{
	const auto all_files = get_unique_files_recursively();

	// create all pairs of shaders
	auto as_vector = std::vector<std::filesystem::path> { all_files.begin(), all_files.end() };
	std::ranges::sort(as_vector);

	Runtime::ShaderCompiler::initialize();

	for (const auto& shader_path : as_vector) {
		if (auto name = shader_path.filename(); shader_cache.contains(name.string())) {
			break;
		}

		const auto relative_path = std::filesystem::relative(shader_path);

		auto shader = Shader::compile(get_device(), relative_path);
		if (!shader) {
			continue;
		}

		auto filename = relative_path.filename().string();
		shader_cache.try_emplace(filename, std::move(shader));
	}
}

} // namespace Disarray
