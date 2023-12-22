#include "DisarrayPCH.hpp"

#include <filesystem>
#include <string>

#include "core/Log.hpp"
#include "core/filesystem/FileIO.hpp"
#include "fmt/core.h"
#include "vulkan/IncludeDirectoryIncluder.hpp"

namespace Disarray {

auto IncludeDirectoryIncluder::includeSystem(const char* header_name, const char* includer_name, size_t inclusion_depth) -> IncResult*
{
	const auto path_to_file = directory / std::filesystem::path { header_name };
	const auto resolved_header_name = std::filesystem::absolute(path_to_file);
	const auto resolved_string = resolved_header_name.string();
	if (includes.contains(resolved_string)) {
		return includes[resolved_string].get();
	}

	if (!std::filesystem::exists(path_to_file)) {
		return &fail_result;
	}

	sources[resolved_string] = {};
	auto& value = sources[resolved_string];

	bool could = FS::read_from_file(resolved_string, value);
	if (!could) {
		return &fail_result;
	}

	auto result = IncludeResultPtr { new IncludeResult { resolved_string, value.c_str(), value.size(), nullptr } };
	auto [it, b] = includes.emplace(resolved_string, std::move(result));
	if (!b) {
		return &fail_result;
	}
	return it->second.get();
}

auto IncludeDirectoryIncluder::includeLocal(const char* header_name, const char* includer_name, size_t inclusion_depth) -> IncResult*
{
	const auto path_to_file = directory / std::filesystem::path { header_name };
	const auto resolved_header_name = std::filesystem::absolute(path_to_file);
	const auto resolved_string = resolved_header_name.string();
	if (includes.contains(resolved_string)) {
		return includes[resolved_string].get();
	}

	if (!std::filesystem::exists(path_to_file)) {
		return &fail_result;
	}

	sources[resolved_string] = {};
	auto& value = sources[resolved_string];

	bool could = FS::read_from_file(resolved_string, value);
	if (!could) {
		return &fail_result;
	}

	auto result = IncludeResultPtr { new IncludeResult { resolved_string, value.c_str(), value.size(), nullptr } };
	auto [it, b] = includes.emplace(resolved_string, std::move(result));
	if (!b) {
		return &fail_result;
	}
	return it->second.get();
}

void IncludeDirectoryIncluder::releaseInclude(IncResult* result)
{
	if (auto iterator = sources.find(result->headerName); iterator != sources.end()) {
		sources.erase(iterator);
	}
	if (auto iterator = includes.find(result->headerName); iterator != includes.end()) {
		includes.erase(iterator);
	}
}

auto IncludeDirectoryIncluder::Deleter::operator()(IncResult* ptr) -> void { delete ptr; }

} // namespace Disarray
