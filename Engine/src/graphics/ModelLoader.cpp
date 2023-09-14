#include "DisarrayPCH.hpp"

#include "graphics/ModelLoader.hpp"

#include <mutex>

#include "core/Collections.hpp"
#include "core/Log.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Device.hpp"
#include "util/Timer.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Texture.hpp"

namespace Disarray {

ModelLoader::ModelLoader(Scope<IModelImporter> input)
	: importer(std::move(input)) {

	};

ModelLoader::ModelLoader(Scope<IModelImporter> input, const std::filesystem::path& path)
	: importer(std::move(input))
{
	import_model(path);
};

auto ModelLoader::import_model(const std::filesystem::path& path) -> void
{
	auto&& meshes = importer->import(path);
	mesh_data = meshes;
}

auto get_mutex() -> auto&
{
	static std::mutex mutex;
	return mutex;
}

void ModelLoader::construct_textures(const Disarray::Device& device)
{
	Timer<float> texture_timer;
	for (auto& [key, value] : mesh_data) {
		Collections::parallel_for_each(value.texture_properties,
			[&dev = device, &texts = value.textures](const TextureProperties& props) { texts.emplace_back(make_ref<Vulkan::Texture>(dev, props)); });
	}
	Log::info("ModelLoader", "Loading textures took {}ms", texture_timer.elapsed<Granularity::Millis>());
}

} // namespace Disarray
