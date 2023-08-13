#pragma once

#include "core/Ensure.hpp"
#include "core/Hashes.hpp"
#include "core/Log.hpp"
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

template <class T>
concept CacheableResource = requires(T t, bool should_clean, const Extent& extent) { t.recreate(should_clean, extent); }
	or requires(T t, bool should_clean, const Extent& extent) { (*t).recreate(should_clean, extent); };

template <CacheableResource Resource, class Props, class Child, class Key = std::string, class Hash = StringHash> class ResourceCache {
	using ResourceMap = std::unordered_map<Key, Resource, Hash, std::equal_to<>>;

public:
	~ResourceCache() { storage.clear(); };

	const Resource& get(const Key& key) const { return storage[key]; }
	Resource& get(const Key& key) { return storage[key]; }

	const Resource& put(const Props& props)
	{
		auto resource = create_from(props);
		const auto& [pair, could] = storage.try_emplace(create_key(props), std::move(resource));
		if (!could)
			Log::error("ResourceCache - Put", "Could not insert resource.");
		return pair->second;
	}

	template <class Func> void for_each_in_storage(Func&& f) { CollectionOperations::for_each(storage, std::forward<Func>(f)); }

	void force_recreate(const Extent& extent) { return get_child().force_recreate_impl(extent); };
	Resource create_from(const Props& props) { return get_child().create_from_impl(props); }
	Key create_key(const Props& props)
	{
		auto key = get_child().create_key_impl(props);
		ensure(!storage.contains(key), fmt::format("Storage already contains key: {}", key));
		return key;
	}

	auto& get_device() { return device; }

protected:
	ResourceCache(Disarray::Device& dev, const std::filesystem::path& p, const std::unordered_set<std::string>& exts = { ".spv", ".png", ".jpg" })
		: device(dev)
		, path(p)
		, extensions(exts)
	{
	}

	std::unordered_set<std::filesystem::path> get_unique_files_recursively() const
	{
		std::unordered_set<std::filesystem::path> paths;
		if (!std::filesystem::exists(path)) {
			Log::error("ResourceCache - Load all files", "{}", "The path was not found.");
			return paths;
		}

		for (const auto& current : std::filesystem::recursive_directory_iterator { path }) {
			const auto has_correct_extension = extensions.contains(current.path().extension().string());
			if (!current.is_regular_file() || !has_correct_extension)
				continue;

			paths.insert(current);
		}
		return paths;
	}

private:
	auto& get_child() { return static_cast<Child&>(*this); }

	Disarray::Device& device;
	ResourceMap storage {};
	std::filesystem::path path {};
	std::unordered_set<std::string> extensions {};
};

} // namespace Disarray
