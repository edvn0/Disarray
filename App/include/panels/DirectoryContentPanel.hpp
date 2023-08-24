#pragma once

#include "core/FileWatcher.hpp"
#include "core/Panel.hpp"
#include "graphics/Texture.hpp"

#include <filesystem>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

struct path_hash {
	std::size_t operator()(const std::optional<std::filesystem::path>& path) const { return path ? std::filesystem::hash_value(path.value()) : 0; }
};

namespace Disarray::Client {

class DirectoryContentPanel : public Panel {
public:
	DirectoryContentPanel(Device&, Window&, Swapchain& sc, const std::filesystem::path& initial = "Assets");
	~DirectoryContentPanel() override = default;

	bool traverse_down(const std::filesystem::path& into_directory, bool force_reload = false);
	bool traverse_up(bool force_reload = false);
	bool can_traverse_up() const;

	void construct(App&, Renderer&, ThreadPool&) override;
	void update(float ts, IGraphicsResource&) override;
	void interface() override;
	void destruct() override;
	void render(Renderer&) override;

	auto& get_current() { return current; }
	std::vector<std::filesystem::path> get_files_in_directory(const std::filesystem::path& for_path) const;
	void draw_file_or_directory(const std::filesystem::path& path, const glm::vec2& size);

private:
	void reload();

	Device& device;
	Scope<FileWatcher> file_watcher {};
	const std::filesystem::path initial;
	std::filesystem::path current {};

	std::filesystem::path previous { initial };
	std::uint32_t depth_from_initial { 0 };
	bool changed { false };

	Ref<Texture> directory_icon { nullptr };
	Ref<Texture> file_icon { nullptr };

	std::mutex mutex;
	std::vector<std::filesystem::path> current_directory_content;

	static constexpr auto icons_max_size = 200;

	std::unordered_map<std::filesystem::path, bool, path_hash> file_type_cache;
	std::unordered_map<std::filesystem::path, std::vector<std::filesystem::path>, path_hash> path_and_content_cache {};
	std::unordered_map<std::filesystem::path, Ref<Texture>, path_hash> icons {};
	std::unordered_set<std::filesystem::path, path_hash> do_not_try_icons {};
};
} // namespace Disarray::Client
