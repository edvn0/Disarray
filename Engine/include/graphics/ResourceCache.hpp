#pragma once

#include "graphics/Device.hpp"

#include <algorithm>
#include <filesystem>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace Disarray {

	namespace CollectionOperations {

		template <typename Collection, typename Func> static inline void for_each(Collection& coll, Func&& func)
		{
			std::ranges::for_each(std::begin(coll), std::end(coll), std::forward<Func>(func));
		}

	} // namespace CollectionOperations

	namespace {
		struct StringHash {
			using is_transparent = void; // enables heterogeneous lookup

			std::size_t operator()(std::string_view sv) const
			{
				std::hash<std::string_view> hasher;
				return hasher(sv);
			}
		};
	} // namespace

	template <class Props, class ResourceProps> struct PropsToResourcePropsMapper {
		ResourceProps operator()(const Props&) = delete;
	};

	template <class Resource, class Props, class ResourceProps, PropsToResourcePropsMapper<Props, ResourceProps> mapper, class Key = std::string,
		class Hash = StringHash>
	class ResourceCache {
		using ResourceMap = std::unordered_map<Key, Ref<Resource>, Hash, std::equal_to<>>;

	public:
		virtual ~ResourceCache() { storage.clear(); };

		virtual void force_recreate();

		virtual const Ref<Resource>& get(const Key&) const = 0;

		virtual const Ref<Resource>& put(const Key& key, const Props& props) { }

	protected:
		ResourceCache(
			Disarray::Device&, const std::filesystem::path&, const std::unordered_set<std::string>& extensions = { ".spv", ".png", ".jpg" });

	private:
		std::unordered_set<std::filesystem::path> get_unique_files_recursively() const
		{
			std::unordered_set<std::filesystem::path> paths;
			for (const auto& current : std::filesystem::recursive_directory_iterator { path }) {
				if (!current.is_regular_file() || current.path().extension() != ".spv")
					continue;

				paths.insert(current);
			}
			return paths;
		}

		ResourceMap storage {};
	};

} // namespace Disarray
