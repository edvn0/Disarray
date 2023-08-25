#pragma once

#include <optional>
#include <stdexcept>
#include <vector>

#include "core/DisarrayObject.hpp"
#include "core/Types.hpp"
#include "core/exceptions/GeneralExceptions.hpp"

namespace Disarray {

class Surface;
class PhysicalDevice;

class QueueFamilyIndex : public ReferenceCountable {
	DISARRAY_OBJECT(QueueFamilyIndex)
public:
	std::uint32_t get_graphics_family() const { return get_or_throw(graphics); };
	std::uint32_t get_compute_family() const { return get_or_throw(compute); };
	std::uint32_t get_transfer_family() const { return get_or_throw(transfer); };
	std::uint32_t get_present_family() const { return get_or_throw(present); };

	operator bool() const { return is_complete(); }

	static Ref<QueueFamilyIndex> construct(Disarray::PhysicalDevice&, Disarray::Surface&);

protected:
	bool is_complete() const { return graphics.has_value() && compute.has_value() && present.has_value(); }
	auto& get_graphics() { return graphics; }
	auto& get_compute() { return compute; }
	auto& get_transfer() { return transfer; }
	auto& get_present() { return present; }

private:
	template <typename T> std::uint32_t get_or_throw(T t) const
	{
		if (!t) {
			throw MissingValueException("Missing value");
		}
		return *t;
	}

	std::optional<std::uint32_t> graphics { std::nullopt };
	std::optional<std::uint32_t> compute { std::nullopt };
	std::optional<std::uint32_t> transfer { std::nullopt };
	std::optional<std::uint32_t> present { std::nullopt };
};

} // namespace Disarray
