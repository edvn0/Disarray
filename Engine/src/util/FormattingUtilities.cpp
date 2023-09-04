#include "DisarrayPCH.hpp"

#include "util/FormattingUtilities.hpp"

#include <fmt/format.h>

namespace Disarray::FormattingUtilities {

namespace Detail {

	auto format_pointer(const void* ptr) -> const void* { return fmt::ptr(ptr); }

} // namespace Detail

} // namespace Disarray::FormattingUtilities
