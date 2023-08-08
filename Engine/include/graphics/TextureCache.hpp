#pragma once

#include "Forward.hpp"
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

	struct TextureCacheCreationProperties : public TextureProperties { };

	class TextureCache {
		struct StringHash {
			using is_transparent = void; // enables heterogeneous lookup

			std::size_t operator()(std::string_view sv) const
			{
				std::hash<std::string_view> hasher;
				return hasher(sv);
			}
		};

		using TextureMap = std::unordered_map<std::string, Ref<Disarray::Texture>, StringHash, std::equal_to<>>;
		using TextureCacheValueType = TextureMap::value_type;

	public:
		TextureCache(Disarray::Device& device, const std::filesystem::path&);
		~TextureCache();

		const Ref<Disarray::Texture>& get(const std::string&);
		const Ref<Disarray::Texture>& put(const TextureCacheCreationProperties&);

		void force_recreation();

	private:
		std::set<std::filesystem::path> get_unique_files_recursively() const;

		Disarray::Device& device;
		std::filesystem::path path { "Assets/Shaders" };

		TextureMap pipeline_cache {};
	};
} // namespace Disarray
