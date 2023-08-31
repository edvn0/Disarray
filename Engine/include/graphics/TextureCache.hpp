#pragma once

#include <algorithm>
#include <filesystem>
#include <set>
#include <unordered_map>
#include <utility>

#include "Forward.hpp"
#include "ResourceCache.hpp"
#include "core/Types.hpp"
#include "graphics/PushConstantLayout.hpp"
#include "graphics/Shader.hpp"
#include "graphics/Swapchain.hpp"
#include "graphics/Texture.hpp"

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
	TextureCache(const Disarray::Device& device, std::filesystem::path path)
		: ResourceCache(device, path, { ".png", ".jpg" })
	{
		auto files = get_unique_files_recursively();
		for (const auto& p : files) {
			put(TextureCacheCreationProperties {
				.key = p.stem().string(),
				.debug_name = fmt::format("TextureCache-{}", p.string()),
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

	auto create_from_impl(const TextureCacheCreationProperties& props) -> Ref<Disarray::Texture>
	{
		return Texture::construct(get_device(),
			TextureProperties {
				.path = props.path.string(),
				.locked_extent = true,
				.debug_name = props.debug_name,
			});
	}

	static auto create_key(const TextureCacheCreationProperties& props) -> std::string { return props.key; }
};
} // namespace Disarray
