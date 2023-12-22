#include "DisarrayPCH.hpp"

#include "core/filesystem/AssetLocations.hpp"

namespace Disarray::FS {
namespace {
	auto default_asset_path() -> auto&
	{
		static std::filesystem::path path { "Assets" };
		return path;
	}
} // namespace

void setup_default_asset_path(std::filesystem::path asset_path) { default_asset_path() = std::move(asset_path); }

auto asset_path() -> std::filesystem::path { return default_asset_path(); }

} // namespace Disarray::FS
