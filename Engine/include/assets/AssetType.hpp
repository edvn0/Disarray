#pragma once

#include <cstdint>
#include <string_view>

namespace Disarray {

enum class AssetFlag : std::uint8_t {
	None = 0,
	Missing = 1 << 1,
	Invalid = 1 << 2,
};
// create bit masking operators (static constexpr) for AssetFla
static constexpr auto operator|(AssetFlag lhs, AssetFlag rhs) -> AssetFlag
{
	return static_cast<AssetFlag>(static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs));
}
static constexpr auto operator&(AssetFlag lhs, AssetFlag rhs) -> AssetFlag
{
	return static_cast<AssetFlag>(static_cast<std::uint8_t>(lhs) & static_cast<std::uint8_t>(rhs));
}
static constexpr auto operator==(AssetFlag lhs, AssetFlag rhs) -> bool
{
	return static_cast<bool>(static_cast<std::uint8_t>(lhs) == static_cast<std::uint8_t>(rhs));
}
static constexpr auto operator!=(AssetFlag lhs, AssetFlag rhs) -> bool
{
	return static_cast<bool>(static_cast<std::uint8_t>(lhs) != static_cast<std::uint8_t>(rhs));
}

enum class AssetType : std::uint16_t {
	None = 0,
	Scene,
	Mesh,
	StaticMesh,
	MeshSource,
	Material,
	Texture,
	EnvMap,
	Font,
	MeshCollider,
};

namespace Utils {

	inline AssetType AssetTypeFromString(std::string_view asset_type)
	{
		if (asset_type == "None")
			return AssetType::None;
		if (asset_type == "Scene")
			return AssetType::Scene;
		if (asset_type == "StaticMesh")
			return AssetType::StaticMesh;
		if (asset_type == "MeshSource")
			return AssetType::MeshSource;
		if (asset_type == "Material")
			return AssetType::Material;
		if (asset_type == "Texture")
			return AssetType::Texture;
		if (asset_type == "EnvMap")
			return AssetType::EnvMap;
		if (asset_type == "Font")
			return AssetType::Font;
		if (asset_type == "MeshCollider")
			return AssetType::MeshCollider;

		return AssetType::None;
	}

	inline const char* AssetTypeToString(AssetType asset_type)
	{
		switch (asset_type) {
		case AssetType::None:
			return "None";
		case AssetType::Scene:
			return "Scene";
		case AssetType::Mesh:
			return "Mesh";
		case AssetType::StaticMesh:
			return "StaticMesh";
		case AssetType::MeshSource:
			return "MeshSource";
		case AssetType::Material:
			return "Material";
		case AssetType::Texture:
			return "Texture";
		case AssetType::EnvMap:
			return "EnvMap";
		case AssetType::Font:
			return "Font";
		case AssetType::MeshCollider:
			return "MeshCollider";
		default:
			return "None";
		}
		return "None";
	}

} // namespace Utils

} // namespace Disarray