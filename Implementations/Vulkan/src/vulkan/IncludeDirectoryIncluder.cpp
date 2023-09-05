#include "DisarrayPCH.hpp"

#include "vulkan/IncludeDirectoryIncluder.hpp"

#include <filesystem>

#include "core/Log.hpp"
#include "core/filesystem/FileIO.hpp"
#include "fmt/core.h"

namespace Disarray {

auto IncludeDirectoryIncluder::includeSystem(const char* header_name, const char* includer_name, size_t inclusion_depth) -> IncResult*
{
	return nullptr;
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
		DISARRAY_LOG_ERROR("IncludeDirectoryIncluder", "Included GLSL shader file '{}' does not exist!", resolved_string);
		return &fail_result;
	}

	sources[resolved_string] = {};

	bool could = FS::read_from_file(resolved_string, sources[resolved_string]);
	if (!could) {
		DISARRAY_LOG_ERROR("IncludeDirectoryIncluder", "Failed to open #included GLSL shader file: {}", resolved_string);
		return &fail_result;
	}

	auto result
		= IncludeResultPtr { new IncludeResult { resolved_string, sources[resolved_string].data(), sources[resolved_string].size(), nullptr } };
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
