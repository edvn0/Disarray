#pragma once

#include "graphics/ImageProperties.hpp"

namespace Disarray {

#define DISARRAY_OBJECT(x)                                                                                                                           \
public:                                                                                                                                              \
	virtual ~x() override = default;                                                                                                                 \
	virtual void recreate(bool, const Extent&) {};                                                                                                   \
	virtual void force_recreation() {};

#define DISARRAY_MAKE_NONCOPYABLE(x)                                                                                                                 \
public:                                                                                                                                              \
	x(const x&) = delete;                                                                                                                            \
	x(x&&) = delete;                                                                                                                                 \
	auto operator=(const x&)->x& = delete;                                                                                                           \
	auto operator=(x&&)->x& = delete;

#define DISARRAY_OBJECT_PROPS(x, p)                                                                                                                  \
public:                                                                                                                                              \
	x() = delete;                                                                                                                                    \
	virtual ~x() override = default;                                                                                                                 \
	DISARRAY_MAKE_NONCOPYABLE(x)                                                                                                                     \
	virtual auto recreate(bool, const Extent&)->void {};                                                                                             \
	virtual auto force_recreation()->void {};                                                                                                        \
                                                                                                                                                     \
	auto get_properties() const->const p& { return props; };                                                                                         \
	auto get_properties()->p& { return props; };                                                                                                     \
                                                                                                                                                     \
protected:                                                                                                                                           \
	x(p properties)                                                                                                                                  \
		: props(std::move(properties))                                                                                                               \
	{                                                                                                                                                \
	}                                                                                                                                                \
	p props; // NOLINT

} // namespace Disarray
