#pragma once

#include "graphics/ImageProperties.hpp"

namespace Disarray {

#define DISARRAY_OBJECT(x)                                                                                                                           \
public:                                                                                                                                              \
	virtual ~x() override = default;                                                                                                                 \
	virtual void recreate(bool, const Extent&) {};                                                                                                   \
	virtual void force_recreation() {};

#define DISARRAY_OBJECT_PROPS(x, props)                                                                                                              \
public:                                                                                                                                              \
	virtual ~x() override = default;                                                                                                                 \
	virtual void recreate(bool, const Extent&) {};                                                                                                   \
	virtual void force_recreation() {};                                                                                                              \
	virtual const props& get_properties() const = 0;                                                                                                 \
	virtual props& get_properties() = 0;

} // namespace Disarray
