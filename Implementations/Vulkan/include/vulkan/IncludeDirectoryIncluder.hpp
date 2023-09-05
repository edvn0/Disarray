#pragma once

#include <glslang/Public/ShaderLang.h>

#include <filesystem>
#include <map>
#include <string>

namespace Disarray {

using IncResult = glslang::TShader::Includer::IncludeResult;
class IncludeDirectoryIncluder final : public glslang::TShader::Includer {
public:
	explicit IncludeDirectoryIncluder(std::filesystem::path dir)
		: directory(std::move(dir))
	{
	}

	// Note "local" vs. "system" is not an "either/or": "local" is an
	// extra thing to do over "system". Both might get called, as per
	// the C++ specification.
	//
	// For the "system" or <>-style includes; search the "system" paths.
	auto includeSystem(const char* header_name, const char* includer_name, size_t inclusion_depth) -> IncResult* override;

	// For the "local"-only aspect of a "" include. Should not search in the
	// "system" paths, because on returning a failure, the parser will
	// call includeSystem() to look in the "system" locations.
	auto includeLocal(const char* header_name, const char* includer_name, size_t inclusion_depth) -> IncResult* override;

	void releaseInclude(IncResult* /*unused*/) override;

private:
	static inline const std::string empty;
	static inline IncludeResult fail_result = { empty, "Header does not exist!", 0, nullptr };

	struct Deleter {
		auto operator()(IncludeResult*) -> void;
	};
	using IncludeResultPtr = std::unique_ptr<IncludeResult, Deleter>;
	std::filesystem::path directory {};
	std::map<std::string, IncludeResultPtr> includes;
	std::map<std::string, std::string> sources;
};

} // namespace Disarray
