#pragma once

#include <algorithm>
#include <filesystem>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "core/Collections.hpp"
#include "core/Ensure.hpp"
#include "core/Hashes.hpp"
#include "core/Log.hpp"
#include "graphics/Device.hpp"
#include "graphics/ImageProperties.hpp"

namespace Disarray {

template <class T>
concept CacheableResource = requires(T t, bool should_clean, const Extent& extent) { t.recreate(should_clean, extent); }
	or requires(T t, bool should_clean, const Extent& extent) { (*t).recreate(should_clean, extent); };

template <CacheableResource Resource, class Props, class Child, class Key = std::string, class Hash = StringHash> class ResourceCache {
	using ResourceMap = std::unordered_map<Key, Resource, Hash, std::equal_to<>, Collections::DefaultAllocator<std::pair<const Key, Resource>>>;
	using UniquePathSet = std::unordered_set<std::filesystem::path, FileSystemPathHasher>;

public:
	~ResourceCache() { clear_storage(); };

	void clear_storage() { storage.clear(); }

	auto get(const Key& key) -> Resource&
	{
		ensure(storage.contains(key), "Key '{}' missing from the resource cache.", key);
		return storage.at(key);
	}

	auto size() const { return storage.size(); }

	auto contains(const Key& key) -> bool { return storage.contains(key); }

	auto put(const Props& props) -> const Resource&
	{
		const auto key = create_key(props);
		if (contains(key)) {
			return get(key);
		}

		auto resource = create_from(props);
		const auto& [pair, could] = storage.try_emplace(std::move(key), std::move(resource));
		return pair->second;
	}

	auto try_put(const Props& props) -> std::tuple<bool, const Resource&>
	{
		const auto key = create_key(props);
		if (contains(key)) {
			return { false, get(key) };
		}

		auto resource = create_from(props);
		const auto& [pair, could] = storage.try_emplace(std::move(key), std::move(resource));
		return { could, pair->second };
	}

	template <class Func> void for_each_in_storage(Func&& func) { Collections::for_each(storage, std::forward<Func>(func)); }

	void force_recreation(const Extent& extent) { return get_child().force_recreation_impl(extent); };
	auto create_from(const Props& props) -> Resource { return get_child().create_from_impl(props); }
	auto create_key(const Props& props) -> Key
	{
		auto key = Child::create_key(props);
		return key;
	}

	[[nodiscard]] auto flatten() -> std::vector<Resource>
	{
		std::vector<Resource> flat {};
		flat.reserve(storage.size());

		for (const auto& [key, value] : storage) {
			flat.push_back(value);
		}

		return flat;
	}

	auto get_device() const -> const auto& { return device; }

protected:
	ResourceCache(
		const Disarray::Device& dev, std::filesystem::path input_path, const std::unordered_set<std::string>& exts = { ".spv", ".png", ".jpg" })
		: device(dev)
		, path(std::move(input_path))
		, extensions(exts)
	{
	}

	[[nodiscard]] auto get_unique_files_recursively(const std::vector<std::filesystem::path>& extra_paths = {}) const -> UniquePathSet
	{
		UniquePathSet paths;
		if (!std::filesystem::exists(path)) {
			return paths;
		}

		for (const auto& current : std::filesystem::recursive_directory_iterator { path }) {
			const auto has_correct_extension = extensions.contains(current.path().extension().string());
			if (!current.is_regular_file() || !has_correct_extension) {
				continue;
			}

			paths.insert(current.path());
		}

		for (const auto& extra_path : extra_paths) {
			for (const auto& current : std::filesystem::recursive_directory_iterator { extra_path }) {
				const auto has_correct_extension = extensions.contains(current.path().extension().string());
				if (!current.is_regular_file() || !has_correct_extension) {
					continue;
				}

				paths.insert(current.path());
			}
		}
		return paths;
	}

private:
	auto get_child() -> auto& { return static_cast<Child&>(*this); }

	const Disarray::Device& device;
	ResourceMap storage {};
	std::filesystem::path path {};
	std::unordered_set<std::string> extensions {};
};

} // namespace Disarray
