#pragma once

#include "Forward.hpp"
#include "core/Types.hpp"

namespace Disarray {

void wait_for_idle(Disarray::Device&);
void wait_for_idle(const Disarray::Device&);

} // namespace Disarray
