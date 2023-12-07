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
class SingleShader;
}

template <> struct std::hash<Disarray::SingleShader> {
	auto operator()(const Disarray::SingleShader& shader) const noexcept -> std::size_t;
};

namespace Disarray {

struct SingleShaderProperties {
	std::filesystem::path path;
	bool optimize { true };
};

class SingleShader : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(SingleShader, SingleShaderProperties)
public:
	virtual auto get_name() const -> std::string_view = 0;
	auto hash() const noexcept -> std::size_t { return std::hash<SingleShader> {}(*this); }
};

} // namespace Disarray
