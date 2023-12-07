#include "DisarrayPCH.hpp"

#include <mutex>

#include "core/Collections.hpp"
#include "core/Log.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Device.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/ModelLoader.hpp"
#include "graphics/ModelVertex.hpp"
#include "graphics/TextureCache.hpp"
#include "util/Timer.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Texture.hpp"

namespace Disarray {

ModelLoader::ModelLoader(Scope<IModelImporter> input)
	: importer(std::move(input)) {

	};

ModelLoader::ModelLoader(Scope<IModelImporter> input, const std::filesystem::path& path, ImportFlag flags)
	: importer(std::move(input))
	, mesh_path(path)
{
	import_model(mesh_path, flags);
};

auto ModelLoader::import_model(const std::filesystem::path& path, ImportFlag flags) -> void
{
	auto&& meshes = importer->import_model(path, flags);
	mesh_data = meshes;
}

auto ModelLoader::construct_textures(const Disarray::Device& device) -> std::vector<Ref<Disarray::Texture>>
{
	TextureCache cache = TextureCache::construct(device, mesh_path.parent_path());
	Timer<float> texture_timer;
	std::size_t saved_iterations = 0;
	for (auto& [key, value] : mesh_data) {
		Collections::for_each(value.texture_properties, [&saved_iterations, &captured = cache](const TextureProperties& props) {
			const auto mip_count = props.generate_mips ? (props.mips.has_value() ? *props.mips : 1) : 1;

			auto&& [could, inserted] = captured.try_put(TextureCacheCreationProperties {
				.key = props.path.string(),
				.debug_name = props.debug_name,
				.path = props.path,
				.mips = mip_count,
				.format = props.format,
			});

			saved_iterations = could ? saved_iterations + 1 : saved_iterations;
		});
	}
	Log::info("ModelLoader", "Loading textures took {}ms. Constructed {} textures. {} copies.", texture_timer.elapsed<Granularity::Millis>(),
		cache.size(), saved_iterations);
	return cache.flatten();
}

} // namespace Disarray
