#include "DisarrayPCH.hpp"

#include "core/Types.hpp"

#include <fmt/format.h>

namespace fmt {

template <std::integral T>
auto fmt::formatter<Disarray::TypeSafeWrapper<T>>::format(const Disarray::TypeSafeWrapper<T>& format, format_context& ctx) -> decltype(ctx.out())
{
	return fmt::formatter<std::string_view>::format(fmt::format("{}", format.operator T()), ctx);
}

auto fmt::formatter<Disarray::TypeSafeWrapper<std::uint32_t>>::format(const Disarray::TypeSafeWrapper<std::uint32_t>& format, format_context& ctx)
	-> decltype(ctx.out())
{
	return fmt::formatter<std::string_view>::format(fmt::format("{}", format.operator std::uint32_t()), ctx);
}

} // namespace fmt
