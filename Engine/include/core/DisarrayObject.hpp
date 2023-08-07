#pragma once

namespace Disarray {

	struct Extent;

#define DISARRAY_OBJECT(x)                                                                                                                           \
public:                                                                                                                                              \
	virtual ~x() override = default;                                                                                                                 \
	virtual void recreate(bool, const Extent&) {};                                                                                                   \
	virtual void force_recreation() {};

} // namespace Disarray
