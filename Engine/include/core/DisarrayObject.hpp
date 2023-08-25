#pragma once

#include "graphics/ImageProperties.hpp"

namespace Disarray {

#define DISARRAY_OBJECT(x)                                                                                                                           \
public:                                                                                                                                              \
	virtual ~x() override = default;                                                                                                                 \
	virtual void recreate(bool, const Extent&) {};                                                                                                   \
	virtual void force_recreation() {};

} // namespace Disarray
