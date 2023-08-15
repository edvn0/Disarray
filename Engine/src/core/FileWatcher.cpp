#include "core/FileWatcher.hpp"

#include <algorithm>
#include <core/Ensure.hpp>

namespace Disarray {

constexpr auto to_filetype(const auto& extension) -> FileType
{
	if (extension == ".txt")
		return FileType::TXT;
	else if (extension == ".png")
		return FileType::PNG;
	else if (extension == ".ttf")
		return FileType::TTF;
	else if (extension == ".jpeg")
		return FileType::JPEG;
	else if (extension == ".jpg")
		return FileType::JPG;
	else if (extension == ".spv")
		return FileType::SPV;
	else if (extension == ".vert")
		return FileType::VERT;
	else if (extension == ".frag")
		return FileType::FRAG;
	else if (extension == ".obj")
		return FileType::OBJ;
	else if (extension == ".json")
		return FileType::JSON;
	else if (extension == ".scene")
		return FileType::SCENE;
	else
		return FileType::UNKNOWN;
}

FileWatcher::FileWatcher(ThreadPool& pool, const std::filesystem::path& in_path, std::chrono::duration<int, std::milli> in_delay)
	: root(in_path)
	, delay(in_delay)
{
	for (const auto& file : std::filesystem::recursive_directory_iterator { root }) {
		const auto path = file.path().string();
		const auto type_if_not_directory = std::filesystem::is_directory(file) ? FileType::DIRECTORY : to_filetype(file.path().extension());

		paths[path] = FileInformation {
			.type = type_if_not_directory,
			.path = path,
			.last_modified = std::filesystem::last_write_time(path),
		};
	}

	finaliser = pool.submit([this]() { loop_until(); });
}

void FileWatcher::loop_until()
{
	while (running) {
		std::this_thread::sleep_for(delay);

		update();
	}
}

void FileWatcher::update()
{

	auto path_iterator = paths.begin();
	while (path_iterator != paths.end()) {
		if (!std::filesystem::exists(path_iterator->first)) {
			path_iterator->second.status = FileStatus::Deleted;
			const auto& current = path_iterator->second;
			for_each(current, activations);
			path_iterator = paths.erase(path_iterator);
		} else {
			path_iterator++;
		}
	}

	for (auto& file : std::filesystem::recursive_directory_iterator(root)) {
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

	for (auto& additional : additional_paths) {
		for (auto& file : std::filesystem::directory_iterator { root / additional }) {
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
}

void FileWatcher::start(const std::function<void(const FileInformation&)>& in) { activations.push_back(in); }

static constexpr auto register_callback(FileStatus status, auto& activations, auto&& function)
{
	auto func = [activation = function, status = status](const auto& file) {
		const auto is_given_status = static_cast<bool>(file.status & status);
		if (is_given_status)
			activation(file);
	};
	activations.push_back(func);
}

void FileWatcher::on_created(const std::function<void(const FileInformation&)>& in) { register_callback(FileStatus::Created, activations, in); }

void FileWatcher::on_modified(const std::function<void(const FileInformation&)>& in) { register_callback(FileStatus::Modified, activations, in); }

void FileWatcher::on_deleted(const std::function<void(const FileInformation&)>& in) { register_callback(FileStatus::Deleted, activations, in); }

void FileWatcher::on_created_or_deleted(const std::function<void(const FileInformation&)>& in)
{
	register_callback(FileStatus::Deleted | FileStatus::Created, activations, in);
}

void FileWatcher::on(FileStatus status, const std::function<void(const FileInformation&)>& in) { register_callback(status, activations, in); }

void FileWatcher::add_watched_paths(const std::filesystem::path& path) { additional_paths.insert(path.string()); }

void FileWatcher::stop()
{
	try {
		running = false;
		finaliser.get();
	} catch (const std::exception& e) {
		Log::error("FileWatcher", "Already joined this thread. Message: {}", e.what());
	}
}

} // namespace Disarray
