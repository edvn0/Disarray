#include "DisarrayPCH.hpp"

#include <fmt/format.h>

#include "core/Types.hpp"

namespace fmt {

template <>
auto fmt::formatter<Disarray::TypeSafeWrapper<std::uint16_t>>::format(const Disarray::TypeSafeWrapper<std::uint16_t>& format, format_context& ctx)
	-> decltype(ctx.out())
{
	return fmt::formatter<std::string_view>::format(fmt::format("{}", format.operator std::uint16_t()), ctx);
}

template <>
auto fmt::formatter<Disarray::TypeSafeWrapper<std::uint32_t>>::format(const Disarray::TypeSafeWrapper<std::uint32_t>& format, format_context& ctx)
	-> decltype(ctx.out())
{
	return fmt::formatter<std::string_view>::format(fmt::format("{}", format.operator std::uint32_t()), ctx);
}

template <>
auto fmt::formatter<Disarray::TypeSafeWrapper<std::int32_t>>::format(const Disarray::TypeSafeWrapper<std::int32_t>& format, format_context& ctx)
	-> decltype(ctx.out())
{
	return fmt::formatter<std::string_view>::format(fmt::format("{}", format.operator std::int32_t()), ctx);
}

} // namespace fmt
