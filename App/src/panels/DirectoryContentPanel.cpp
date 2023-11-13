#include "panels/DirectoryContentPanel.hpp"

#include <Disarray.hpp>

#include <filesystem>
#include <string_view>

#include "graphics/ImageProperties.hpp"
#include "graphics/TextureCache.hpp"
#include "ui/UI.hpp"

namespace Disarray::Client {

using namespace std::string_view_literals;

DirectoryContentPanel::DirectoryContentPanel(Device&, Window&, Swapchain&, const std::filesystem::path& initial, Disarray::TextureCache& cache)
	: texture_cache(cache)
	, initial(initial)
	, current(initial)
{
}

auto DirectoryContentPanel::traverse_down(const std::filesystem::path& into_directory, bool force_reload) -> bool
{
	bool could = true;

	if (!std::filesystem::exists(into_directory)) {
		could = false;
	}

	std::swap(previous, current);
	current = into_directory;

	if (current != previous) {
		changed = true;
	}

	if (!path_and_content_cache.contains(current)) {
		path_and_content_cache[current] = get_files_in_directory(current);
	} else {
		if (force_reload) {
			path_and_content_cache[current] = get_files_in_directory(current);
		}
	}

	if (could) {
		depth_from_initial++;
	}
	return could;
}

auto DirectoryContentPanel::traverse_up(bool force_reload) -> bool
{
	bool could = true;

	if (!std::filesystem::exists(previous)) {
		could = false;
	}

	if (current == initial) {
		previous = initial;
	}

	std::swap(current, previous);

	if (current != previous) {
		changed = true;
	}

	if (!path_and_content_cache.contains(current)) {
		path_and_content_cache[current] = get_files_in_directory(current);
	} else {
		if (force_reload) {
			path_and_content_cache[current] = get_files_in_directory(current);
		}
	}

	if (could) {
		depth_from_initial--;
	}
	return could;
}

void DirectoryContentPanel::update(float)
{
	if (changed) {
		current_directory_content = path_and_content_cache[current];
		changed = false;
	}
}

auto DirectoryContentPanel::can_traverse_up() const -> bool { return current != initial; }

void DirectoryContentPanel::interface()
{
	bool force_reload = false;
	if (Input::all<KeyCode::LeftControl, KeyCode::LeftShift, KeyCode::R>()) {
		reload();
		force_reload = true;
	}

	ImGui::Begin("Directory Content");

	if (can_traverse_up() && ImGui::ArrowButton("##GoBack", ImGuiDir_Left)) {
		traverse_up(force_reload);
	}
	ImGui::SameLine();
	if (ImGui::ArrowButton("##GoUpOneLevel", ImGuiDir_Up)) {
		auto parent = current.parent_path();
		static auto initials_parent = initial.parent_path();
		if (std::filesystem::exists(parent) && parent != initials_parent) {
			current = parent;
			changed = true;
		}
	}

	static float padding = 16.0F;
	static float thumbnail_size = 64.0F;
	float cell_size = thumbnail_size + padding;

	float panel_width = ImGui::GetContentRegionAvail().x;
	auto column_count = static_cast<int>(panel_width / cell_size);
	if (column_count < 1) {
		column_count = 1;
	}

	if (ImGui::BeginTable("DirectoryContent", column_count)) {
		for (const auto& directory_entry : current_directory_content) {
			const auto& path = directory_entry;
			const auto filename = path.filename();
			std::string filename_string = filename.string();
			ImGui::PushID(path.c_str());

			const auto path_is_image = is_image(path);

			if (!path_is_image) {
				draw_file_or_directory(path, { thumbnail_size, thumbnail_size });
			} else {
				const auto exists = texture_cache.put({
					.key = filename.string(),
					.debug_name = fmt::format("DirectoryContentPanelImage-{}", path.filename().string()),
					.path = path.string(),
				});

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				UI::image(*texture_cache.get(filename.string()), { thumbnail_size, thumbnail_size });
				ImGui::PopStyleColor();
			}

			UI::drag_drop(path);
			UI::handle_double_click([&directory_entry, force_reload, this] {
				if (is_directory(directory_entry)) {
					traverse_down(directory_entry, force_reload);
				}
			});

			UI::text_wrapped("{}", filename_string);

			ImGui::TableNextColumn();

			ImGui::PopID();
		}
		ImGui::EndTable();
	}

	ImGui::End();
}

void DirectoryContentPanel::construct(App&)
{
	using namespace std::chrono_literals;

	auto& pool = App::get_thread_pool();

	file_watcher = make_scope<FileWatcher>(pool, "Assets", 200ms);
	path_and_content_cache[current] = get_files_in_directory(current);
	current_directory_content = path_and_content_cache[current];
	directory_icon = texture_cache.get("Directory");
	file_icon = texture_cache.get("File");

	file_watcher->on(FileStatuses::All, [this](const FileInformation& file_info) {
		std::filesystem::path file_path { file_info.path };
		if (const auto file_directory_for_modified_file = file_path.parent_path(); file_directory_for_modified_file != current) {
			return;
		}

		std::unique_lock lock { mutex };
		reload();
	});
}

void DirectoryContentPanel::reload()
{
	path_and_content_cache[current] = get_files_in_directory(current);
	current_directory_content = path_and_content_cache[current];
}

void DirectoryContentPanel::destruct()
{
	file_watcher.reset();
	current_directory_content.clear();
	path_and_content_cache.clear();
}

auto DirectoryContentPanel::get_files_in_directory(const std::filesystem::path& for_path) const -> std::vector<std::filesystem::path>
{
	std::vector<std::filesystem::path> found;
	for (const auto& entry : std::filesystem::directory_iterator { for_path }) {
		found.push_back(entry.path());
	}
	return found;
}

void DirectoryContentPanel::draw_file_or_directory(const std::filesystem::path& path, const glm::vec2& size)
{
	if (!file_type_cache.contains(path)) {
		file_type_cache[path] = is_directory(path);
	}
	auto& icon = file_type_cache[path] ? directory_icon : file_icon;
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	UI::image(*icon, size);
	ImGui::PopStyleColor();
}

} // namespace Disarray::Client
