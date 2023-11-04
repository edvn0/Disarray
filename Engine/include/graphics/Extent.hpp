#pragma once

#include <cmath>

#include "core/Concepts.hpp"

namespace Disarray {

template <IsNumber T> struct IExtent {
	T width {};
	T height {};

	IExtent(T w, T h)
		: width(w)
		, height(h)
	{
	}

	IExtent(std::integral auto w, std::integral auto h)
		: width(static_cast<T>(w))
		, height(static_cast<T>(h))
	{
	}

	IExtent() = default;
	IExtent(const IExtent& other) = default;
	IExtent(IExtent&& other) noexcept = default;

	[[nodiscard]] auto get_size() const -> T { return width * height; }
	[[nodiscard]] auto aspect_ratio() const -> float { return static_cast<float>(width) / static_cast<float>(height); }

	[[nodiscard]] auto valid() const -> bool { return width > 0 && height > 0; }

	auto operator=(const IExtent<T>& other) -> IExtent<T>&
	{
		width = other.width;
		height = other.height;
		return *this;
	}
	template <IsNumber Other> auto operator=(const IExtent<Other>& other) -> IExtent<T>&
	{
		width = static_cast<T>(other.width);
		height = static_cast<T>(other.height);
		return *this;
	}

	auto operator=(IExtent<T>&& other) -> IExtent<T>&
	{
		width = std::move(other.width);
		height = std::move(other.height);
		return *this;
	}
	template <IsNumber Other> auto operator=(IExtent<Other>&& other) -> IExtent<T>&
	{
		width = static_cast<T>(std::move(other.width));
		height = static_cast<T>(std::move(other.height));
		return *this;
	}

	auto operator==(const IExtent<T>& other) const -> bool
	{
		return compare_operation(width, other.width) && compare_operation(height, other.height);
	}
	auto operator!=(const IExtent<T>& other) const -> bool
	{
		return !compare_operation(width, other.width) || !compare_operation(height, other.height);
	}

	template <IsNumber Other> auto operator==(const IExtent<Other>& other) const -> bool
	{
		return compare_operation(width, other.width) && compare_operation(height, other.height);
	}
	template <IsNumber Other> auto operator!=(const IExtent<Other>& other) const -> bool
	{
		return !compare_operation(width, other.width) || !compare_operation(height, other.height);
	}

	template <IsNumber Other> auto operator*=(const IExtent<Other>& other) -> IExtent<T>& { return operator*=(other.template as<T>()); }
	auto operator*=(const IExtent<T>& other) -> IExtent<T>&
	{
		width *= other.width;
		height *= other.height;
		return *this;
	}

	template <IsNumber Other> auto operator*(const IExtent<Other>& other) -> IExtent<T> { return operator*(other.template as<T>()); }
	auto operator*(const IExtent<T>& other) -> IExtent<T>
	{
		return IExtent<T> {
			.width = width * other.width,
			.height = height * other.height,
		};
	}

	template <IsNumber Other> auto operator*(Other other) -> IExtent<T> { return operator*(static_cast<T>(other)); }
	auto operator*(T other) -> IExtent<T>
	{
		return IExtent<T> {
			.width = width * other,
			.height = height * other,
		};
	}

	template <IsNumber Other> auto operator/=(const IExtent<Other>& other) -> IExtent<T>& { return operator/=(other.template as<T>()); }
	auto operator/=(const IExtent<T>& other) -> IExtent<T>&
	{
		width *= other.width;
		height *= other.height;
		return *this;
	}

	template <IsNumber Other> auto operator/=(Other other) -> IExtent<T>& { return operator/=(other); }
	auto operator/=(T other) -> IExtent<T>&
	{
		width /= other;
		height /= other;
		return *this;
	}

	constexpr auto compare_operation(std::floating_point auto left, std::floating_point auto right) const
	{
		return std::fabs(left - right) < std::numeric_limits<float>::epsilon();
	}

	constexpr auto compare_operation(std::integral auto left, std::floating_point auto right) const
	{
		return std::fabs(static_cast<decltype(right)>(left) - right) < std::numeric_limits<float>::epsilon();
	}

	constexpr auto compare_operation(std::floating_point auto left, std::integral auto right) const
	{
		return std::fabs(left - static_cast<decltype(left)>(right)) < std::numeric_limits<float>::epsilon();
	}

	constexpr auto compare_operation(std::integral auto left, std::integral auto right) const { return left == right; }

	auto to_string() -> std::string { return fmt::format("{}:{}", width, height); }

	[[nodiscard]] auto sum() const -> float { return width + height; }

	template <IsNumber Other>
		requires(!std::is_same_v<T, Other>)
	auto as() const -> IExtent<Other>
	{
		return IExtent<Other> {
			static_cast<Other>(width),
			static_cast<Other>(height),
		};
	}

	template <typename VecType>
		requires requires(VecType vector) {
			vector.x;
			vector.y;
		}
	auto as() const -> VecType
	{
		return VecType {
			static_cast<float>(width),
			static_cast<float>(height),
		};
	}

	template <IsNumber Other>
		requires(!std::is_same_v<T, Other>)
	[[nodiscard]] auto cast() const
	{
		return std::pair {
			static_cast<Other>(width),
			static_cast<Other>(height),
		};
	}
};

using Extent = IExtent<std::uint32_t>;
using FloatExtent = IExtent<float>;

} // namespace Disarray
