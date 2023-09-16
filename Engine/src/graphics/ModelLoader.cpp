#include "DisarrayPCH.hpp"

#include "graphics/ModelLoader.hpp"

#include <mutex>

#include "core/Collections.hpp"
#include "core/Log.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Device.hpp"
#include "graphics/TextureCache.hpp"
#include "util/Timer.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Texture.hpp"

namespace Disarray {

ModelLoader::ModelLoader(Scope<IModelImporter> input)
	: importer(std::move(input)) {

	};

ModelLoader::ModelLoader(Scope<IModelImporter> input, const std::filesystem::path& path)
	: importer(std::move(input))
	, mesh_path(path)
{
	import_model(path);
};

auto ModelLoader::import_model(const std::filesystem::path& path) -> void
{
	auto&& meshes = importer->import(path);
	mesh_data = meshes;
}

void ModelLoader::construct_textures(const Disarray::Device& device)
{
	TextureCache cache { device, mesh_path.parent_path() };
	Timer<float> texture_timer;
	for (auto& [key, value] : mesh_data) {
		Collections::for_each(value.texture_properties, [&captured = cache, &texts = value.textures](const TextureProperties& props) {
			texts.push_back(captured.put(TextureCacheCreationProperties {
				.key = props.path.string(),
				.debug_name = props.debug_name,
				.path = props.path,
				.mips = *props.mips,
				.format = props.format,
			}));
		});
	}
	Log::info("ModelLoader", "Loading textures took {}ms", texture_timer.elapsed<Granularity::Millis>());
}

} // namespace Disarray
