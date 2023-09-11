#pragma once

#include <chrono>
#include <execution>
#include <filesystem>
#include <functional>
#include <future>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "core/Collections.hpp"
#include "core/Hashes.hpp"
#include "core/ThreadPool.hpp"

namespace Disarray {

inline constexpr auto bit(auto x) { return 1 << x; }

enum class FileType : std::uint8_t { DIRECTORY, UNKNOWN, TXT, PNG, TTF, JPEG, JPG, SPV, VERT, FRAG, OBJ, JSON, SCENE };
enum class FileStatus : std::uint8_t { Created = bit(0), Deleted = bit(1), Modified = bit(2) };

static constexpr auto operator|(FileStatus left, FileStatus right)
{
	return static_cast<FileStatus>(static_cast<std::uint8_t>(left) | static_cast<std::uint8_t>(right));
}

static constexpr auto operator&(FileStatus left, FileStatus right)
{
	return static_cast<FileStatus>(static_cast<std::uint8_t>(left) & static_cast<std::uint8_t>(right));
}

namespace FileStatuses {
	static constexpr FileStatus C = FileStatus::Created;
	static constexpr FileStatus D = FileStatus::Deleted;
	static constexpr FileStatus M = FileStatus::Modified;
	static constexpr FileStatus CD = FileStatus::Created | FileStatus::Deleted;
	static constexpr FileStatus CM = FileStatus::Created | FileStatus::Modified;
	static constexpr FileStatus DM = FileStatus::Deleted | FileStatus::Modified;
	static constexpr FileStatus All = FileStatus::Created | FileStatus::Deleted | FileStatus::Modified;
} // namespace FileStatuses

struct FileInformation {
	FileType type;
	std::string path;
	std::filesystem::file_time_type last_modified;
	FileStatus status = FileStatus::Created;

	[[nodiscard]] auto to_path() const { return std::filesystem::path { path }; }
	[[nodiscard]] auto is_valid() const { return std::filesystem::is_regular_file(to_path()); }
	[[nodiscard]] auto has_extension(std::string_view ext) const { return to_path().extension().string() == ext; }
};

struct FileWatcherCallback {
	auto operator()(const FileInformation& info) -> void;
};

class FileWatcher {
public:
	FileWatcher(Threading::ThreadPool&, const std::filesystem::path&, std::chrono::duration<int, std::milli> = std::chrono::milliseconds(2000));
	FileWatcher(Threading::ThreadPool&, const std::filesystem::path&, const Collections::StringSet& extensions,
		std::chrono::duration<int, std::milli> = std::chrono::milliseconds(2000));
	~FileWatcher();

	void on_created(const std::function<void(const FileInformation&)>& activation_function);
	void on_created_or_modified(const std::function<void(const FileInformation&)>& activation_function);
	void on_modified(const std::function<void(const FileInformation&)>& activation_function);
	void on_deleted(const std::function<void(const FileInformation&)>& activation_function);
	void on_created_or_deleted(const std::function<void(const FileInformation&)>& activation_function);
	void on(FileStatus info, const std::function<void(const FileInformation&)>& activation_function);

private:
	static void for_each(const FileInformation& file_information, const std::vector<std::function<void(const FileInformation&)>>& funcs)
	{
		Collections::for_each(funcs, [&info = file_information](auto& func) { func(info); });
	}

	void start(const std::function<void(const FileInformation&)>& activation_function);
	void stop();
	void loop_until();
	void update();

	auto in_extensions(const auto& entry)
	{
		if (extensions.contains("*")) {
			return true;
		}

		if (extensions.contains(entry.path().extension().string())) {
			return true;
		}
		return false;
	}

	std::vector<std::function<void(const FileInformation&)>> activations;
	std::filesystem::path root;
	Collections::StringSet extensions {};
	std::chrono::duration<int, std::milli> delay;
	Collections::StringMap<FileInformation> paths {};
	std::atomic_bool running { true };
	std::future<void> finaliser;
};

} // namespace Disarray
