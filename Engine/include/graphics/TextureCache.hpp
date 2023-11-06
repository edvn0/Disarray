#pragma once

#include "Forward.hpp"

#include <algorithm>
#include <filesystem>
#include <set>
#include <unordered_map>
#include <utility>

#include "core/Types.hpp"
#include "graphics/PushConstantLayout.hpp"
#include "graphics/ResourceCache.hpp"
#include "graphics/Shader.hpp"
#include "graphics/Swapchain.hpp"
#include "graphics/Texture.hpp"

namespace Disarray {

struct TextureCacheCreationProperties {
	std::string key;
	std::string debug_name;
	std::filesystem::path path;
	std::uint32_t mips { 1 };
	ImageFormat format { ImageFormat::SRGB };
};

class TextureCache : public ResourceCache<Ref<Disarray::Texture>, TextureCacheCreationProperties, TextureCache, std::string, StringHash> {
public:
	TextureCache(const Disarray::Device& device, std::filesystem::path path, bool load_files = true)
		: ResourceCache(device, std::move(path), { ".png", ".jpg" })
	{
		if (load_files) {
			auto files = get_unique_files_recursively({ "Assets/Icons" });
			for (const auto& file_path : files) {
				put(TextureCacheCreationProperties {
					.key = file_path.stem().string(),
					.debug_name = fmt::format("TextureCache-{}", file_path.string()),
					.path = file_path.string(),
				});
			}
		}
	}

	static auto construct(const Disarray::Device& device, std::filesystem::path path) { return TextureCache { device, path, false }; }

	void force_recreation_impl(const Extent& extent)
	{
		for_each_in_storage([&extent](auto& resource) {
			auto& [k, v] = resource;
			v->recreate(true, extent);
		});
	}

	auto create_from_impl(const TextureCacheCreationProperties& props) -> Ref<Disarray::Texture>
	{
		if (contains(props.key)) {
			return get(props.key);
		}

		return Texture::construct(ResourceCache::get_device(),
			TextureProperties {
				.generate_mips = true,
				.mips = props.mips,
				.path = props.path.string(),
				.locked_extent = true,
				.debug_name = props.debug_name,
			});
	}

	static auto create_key(const TextureCacheCreationProperties& props) -> std::string { return props.key; }
};
} // namespace Disarray
