#pragma once

#include <fmt/core.h>

#include <filesystem>
#include <optional>
#include <string_view>
#include <vector>

#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/Types.hpp"

namespace Disarray {

struct UnifiedShaderProperties {
	std::filesystem::path path;
	bool optimize { true };
};

class UnifiedShader : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(UnifiedShader, UnifiedShaderProperties)
};

} // namespace Disarray

template <> struct std::hash<Disarray::UnifiedShader> {
	auto operator()(const Disarray::UnifiedShader& shader) const noexcept -> std::size_t { return hash_value(shader.get_properties().path); }
};
