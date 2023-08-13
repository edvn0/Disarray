#pragma once

#include "Forward.hpp"
#include "ResourceCache.hpp"
#include "core/Types.hpp"
#include "graphics/PushConstantLayout.hpp"
#include "graphics/Shader.hpp"
#include "graphics/Swapchain.hpp"
#include "graphics/Texture.hpp"

#include <algorithm>
#include <filesystem>
#include <set>
#include <unordered_map>
#include <utility>

namespace Disarray {

struct TextureCacheCreationProperties {
	std::string key;
	std::string debug_name;
	std::filesystem::path path;
	std::uint32_t mips { 1 };
	ImageFormat format;
};

class TextureCache : public ResourceCache<Ref<Disarray::Texture>, TextureCacheCreationProperties, TextureCache, std::string, StringHash> {
public:
	TextureCache(Disarray::Device& device, std::filesystem::path path)
		: ResourceCache(device, path, { ".png", ".jpg" })
	{
		auto files = get_unique_files_recursively();
		for (const auto& p : files) {
			put(TextureCacheCreationProperties {
				.key = p.stem().string(),
				.path = p.string(),
			});
		}
	}

	void force_recreate_impl(const Extent& extent)
	{
		for_each_in_storage([&extent](auto& resource) {
			auto& [k, v] = resource;
			v->recreate(true, extent);
		});
	}

	Ref<Disarray::Texture> create_from_impl(const TextureCacheCreationProperties& props)
	{
		return Texture::construct(get_device(),
			TextureProperties {
				.path = props.path.string(),
			});
	}

	std::string create_key_impl(const TextureCacheCreationProperties& props) { return props.key; }
};
} // namespace Disarray
