#pragma once

#include <optional>

#include "core/DisarrayObject.hpp"
#include "core/PointerDefinition.hpp"
#include "core/exceptions/GeneralExceptions.hpp"

namespace Disarray {

class Surface;
class PhysicalDevice;

class QueueFamilyIndex : public ReferenceCountable {
	DISARRAY_OBJECT_NO_PROPS(QueueFamilyIndex)
public:
	auto get_graphics_family() const -> std::uint32_t { return get_or_throw(graphics); };
	auto get_compute_family() const -> std::uint32_t { return get_or_throw(compute); };
	auto get_transfer_family() const -> std::uint32_t { return get_or_throw(transfer); };
	auto get_present_family() const -> std::uint32_t { return get_or_throw(present); };

	operator bool() const { return is_complete(); }

	static auto construct(Disarray::PhysicalDevice&, Disarray::Surface&) -> Ref<Disarray::QueueFamilyIndex>;

protected:
	auto is_complete() const -> bool { return graphics.has_value() && compute.has_value() && present.has_value(); }
	auto get_graphics() -> auto& { return graphics; }
	auto get_compute() -> auto& { return compute; }
	auto get_transfer() -> auto& { return transfer; }
	auto get_present() -> auto& { return present; }

private:
	template <typename T> auto get_or_throw(T input) const -> std::uint32_t
	{
		if (!input) {
			throw MissingValueException("Missing value");
		}
		return *input;
	}

	std::optional<std::uint32_t> graphics { std::nullopt };
	std::optional<std::uint32_t> compute { std::nullopt };
	std::optional<std::uint32_t> transfer { std::nullopt };
	std::optional<std::uint32_t> present { std::nullopt };
};

} // namespace Disarray
