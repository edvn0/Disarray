#include "core/FileWatcher.hpp"

#include <core/Ensure.hpp>
#include <magic_enum.hpp>

#include <algorithm>

namespace Disarray {

constexpr auto to_filetype(const auto& extension) -> FileType
{
	if (extension == ".txt") {
		return FileType::TXT;
	}
	if (extension == ".png") {
		return FileType::PNG;
	}
	if (extension == ".ttf") {
		return FileType::TTF;
	}
	if (extension == ".jpeg") {
		return FileType::JPEG;
	}
	if (extension == ".jpg") {
		return FileType::JPG;
	}
	if (extension == ".spv") {
		return FileType::SPV;
	}
	if (extension == ".vert") {
		return FileType::VERT;
	}
	if (extension == ".frag") {
		return FileType::FRAG;
	}
	if (extension == ".obj") {
		return FileType::OBJ;
	}
	if (extension == ".json") {
		return FileType::JSON;
	}
	if (extension == ".scene") {
		return FileType::SCENE;
	}
	return FileType::UNKNOWN;
}

FileWatcher::FileWatcher(Threading::ThreadPool& pool, const std::filesystem::path& in_path, std::chrono::duration<int, std::milli> in_delay)
	: FileWatcher(pool, in_path, { "*" }, in_delay)
{
}

FileWatcher::FileWatcher(Threading::ThreadPool& pool, const std::filesystem::path& in_path, const Collections::StringSet& exts,
	std::chrono::duration<int, std::milli> in_delay)
	: root(in_path)
	, extensions(exts)
	, delay(in_delay)
{
	for (const auto& file : std::filesystem::recursive_directory_iterator { root }) {
		if (!in_extensions(file)) {
			continue;
		}

		const auto path = file.path().string();
		const auto type_if_not_directory = std::filesystem::is_directory(file) ? FileType::DIRECTORY : to_filetype(file.path().extension());

		paths[path] = FileInformation {
			.type = type_if_not_directory,
			.path = path,
			.last_modified = std::filesystem::last_write_time(path),
		};
	}

	finaliser = pool.submit(&FileWatcher::loop_until, this);
}

void FileWatcher::loop_until()
{
	while (running) {
		std::this_thread::sleep_for(delay);
		update();
	}
}

namespace {
	auto check_for_delete(auto& paths, auto& activations, auto* func)
	{
		auto path_iterator = paths.begin();
		while (path_iterator != paths.end()) {
			if (!std::filesystem::exists(path_iterator->first)) {
				path_iterator->second.status = FileStatus::Deleted;
				const auto& current = path_iterator->second;
				func(current, activations);
				path_iterator = paths.erase(path_iterator);
			} else {
				path_iterator++;
			}
		}
	}

	auto register_callback(FileStatus status, auto& activations, auto&& function)
	{
		auto func = [activation = function, status = status](const FileInformation& file) {
			const auto is_given_status = (file.status & status) != FileStatus {};
			if (is_given_status) {
				Log::info("FileWatcher", "Status for file '{}': Old status: '{}', New status: '{}'", file.path, magic_enum::enum_name(file.status),
					magic_enum::enum_name(status));
				activation(file);
			}
		};
		activations.push_back(func);
	}

} // namespace

void FileWatcher::update()
{
	check_for_delete(paths, activations, &for_each);

	for (const auto& file : std::filesystem::recursive_directory_iterator(root)) {
		if (!in_extensions(file)) {
			continue;
		}

		auto current_file_last_write_time = std::filesystem::last_write_time(file);
		const auto view = file.path().string();

		if (!paths.contains(view)) {
			const auto type_if_not_directory = std::filesystem::is_directory(file) ? FileType::DIRECTORY : to_filetype(file.path().extension());

			paths[file.path().string()] = FileInformation {
				.type = type_if_not_directory, .path = view, .last_modified = current_file_last_write_time, .status = FileStatus::Created
			};

			const auto& current = paths[file.path().string()];
			for_each(current, activations);
		} else {
			auto& current = paths[file.path().string()];

			if (current.last_modified != current_file_last_write_time) {
				current.last_modified = current_file_last_write_time;
				current.status = FileStatus::Modified;
				for_each(current, activations);
			}
		}
	}
}

void FileWatcher::start(const std::function<void(const FileInformation&)>& activation_function) { activations.push_back(activation_function); }

void FileWatcher::on_created(const std::function<void(const FileInformation&)>& activation_function)
{
	register_callback(FileStatus::Created, activations, activation_function);
}

void FileWatcher::on_modified(const std::function<void(const FileInformation&)>& activation_function)
{
	register_callback(FileStatus::Modified, activations, activation_function);
}
void FileWatcher::on_created_or_modified(const std::function<void(const FileInformation&)>& activation_function)
{
	register_callback(FileStatus::Modified | FileStatus::Created, activations, activation_function);
}

void FileWatcher::on_deleted(const std::function<void(const FileInformation&)>& activation_function)
{
	register_callback(FileStatus::Deleted, activations, activation_function);
}

void FileWatcher::on_created_or_deleted(const std::function<void(const FileInformation&)>& activation_function)
{
	register_callback(FileStatus::Deleted | FileStatus::Created, activations, activation_function);
}

void FileWatcher::on(FileStatus info, const std::function<void(const FileInformation&)>& activation_function)
{
	register_callback(info, activations, activation_function);
}

FileWatcher::~FileWatcher() { stop(); }

void FileWatcher::stop()
{
	running = false;
	finaliser.wait();
	activations.clear();
}

} // namespace Disarray
