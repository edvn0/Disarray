#pragma once

#include "Forward.hpp"

#include "core/Types.hpp"
#include "graphics/Extent.hpp"

namespace Disarray {

#define DISARRAY_MAKE_NONCOPYABLE(Type)                                                                                                              \
public:                                                                                                                                              \
	Type(const Type&) = delete;                                                                                                                      \
	Type(Type&&) = delete;                                                                                                                           \
	auto operator=(const Type&)->Type& = delete;                                                                                                     \
	auto operator=(Type&&)->Type& = delete;

#define DISARRAY_OBJECT_NO_PROPS(Type)                                                                                                               \
public:                                                                                                                                              \
	Type() = default;                                                                                                                                \
	virtual ~Type() override = default;                                                                                                              \
	DISARRAY_MAKE_NONCOPYABLE(Type)                                                                                                                  \
	virtual auto recreate(bool, const Extent&)->void {};                                                                                             \
	virtual auto force_recreation()->void {};

#define DISARRAY_OBJECT_PROPS(Type, PropertiesType)                                                                                                  \
public:                                                                                                                                              \
	virtual ~Type() override = default;                                                                                                              \
	DISARRAY_MAKE_NONCOPYABLE(Type)                                                                                                                  \
	virtual auto recreate(bool, const Extent&)->void {};                                                                                             \
	virtual auto force_recreation()->void {};                                                                                                        \
	auto get_properties() const->const PropertiesType& { return props; };                                                                            \
	auto get_properties()->PropertiesType& { return props; };                                                                                        \
	static auto construct(const Disarray::Device& device, PropertiesType properties)->Ref<Disarray::Type>;                                           \
	static auto construct_scoped(const Disarray::Device& device, PropertiesType properties)->Scope<Disarray::Type>;                                  \
                                                                                                                                                     \
protected:                                                                                                                                           \
	Type() = default;                                                                                                                                \
	explicit Type(PropertiesType properties)                                                                                                         \
		: props(std::move(properties))                                                                                                               \
	{                                                                                                                                                \
	}                                                                                                                                                \
	PropertiesType props; // NOLINT

} // namespace Disarray
