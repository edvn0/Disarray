#pragma once

#include "assets/AssetType.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/UniquelyIdentifiable.hpp"

namespace Disarray {

using AssetHandle = Identifier;

class Asset : public ReferenceCountable {
public:
	AssetHandle Handle = 0;
	AssetFlag Flags = AssetFlag::None;

	virtual ~Asset() { }

	[[nodiscard]] static AssetType get_static_type() { return AssetType::None; }
	virtual AssetType get_asset_type() const { return AssetType::None; }

	auto is_valid() -> bool const
	{
		// Should return true if the Flags member is not invalid or not missing.
		return Flags != AssetFlag::Invalid && Flags != AssetFlag::Missing;
	}

	auto operator==(const Asset& other) const -> bool { return Handle == other.Handle; }
	auto operator!=(const Asset& other) const -> bool { return Handle != other.Handle; }
};

} // namespace Disarray