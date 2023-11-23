#pragma once

#include "assets/Asset.hpp"
#include "core/Ensure.hpp"
#include "graphics/Material.hpp"

namespace Disarray {

class MaterialAsset : public Asset {
public:
	explicit MaterialAsset(bool transparent);
	explicit MaterialAsset(Ref<Material> material);
	~MaterialAsset() override;

	glm::vec3& GetAlbedoColor();
	void SetAlbedoColor(const glm::vec3& color);

	float& GetMetalness();
	void SetMetalness(float value);

	float& GetRoughness();
	void SetRoughness(float value);

	float& GetEmission();
	void SetEmission(float value);

	// Textures
	Ref<Texture> GetAlbedoMap();
	void SetAlbedoMap(Ref<Texture> texture);
	void ClearAlbedoMap();

	Ref<Texture> GetNormalMap();
	void SetNormalMap(Ref<Texture> texture);
	bool IsUsingNormalMap();
	void SetUseNormalMap(bool value);
	void ClearNormalMap();

	Ref<Texture> GetMetalnessMap();
	void SetMetalnessMap(Ref<Texture> texture);
	void ClearMetalnessMap();

	Ref<Texture> GetRoughnessMap();
	void SetRoughnessMap(Ref<Texture> texture);
	void ClearRoughnessMap();

	float& GetTransparency();
	void SetTransparency(float transparency);

	[[nodiscard]] static auto get_static_type() -> AssetType { return AssetType::Material; }
	auto get_asset_type() const -> AssetType override { return get_static_type(); }

	Ref<Material> get_material() const { return material; }
	void set_material(const Ref<Material>& input_material) { material = input_material; }

	bool is_transparent() const { return enabled_transparent; }

private:
	void set_default();

private:
	Ref<Material> material;
	bool enabled_transparent = false;
};

class MaterialTable : public ReferenceCountable {
private:
	explicit MaterialTable(std::uint32_t new_material_count);
	explicit MaterialTable(const Ref<MaterialTable>& other);

public:
	~MaterialTable() override = default;

	bool has_material(std::uint32_t material_index) const { return materials.contains(material_index); }
	void set_material(std::uint32_t index, Ref<MaterialAsset> material);
	void clear_material(std::uint32_t index);

	Ref<MaterialAsset> get_material(std::uint32_t material_index) const
	{
		ensure(has_material(material_index));
		return materials.at(material_index);
	}
	auto get_materials() -> std::unordered_map<std::uint32_t, Ref<MaterialAsset>>& { return materials; }
	auto get_materials() const -> const std::unordered_map<std::uint32_t, Ref<MaterialAsset>>& { return materials; }

	std::uint32_t get_material_count() const { return material_count; }
	void set_material_count(std::uint32_t new_material_count) { material_count = new_material_count; }

	void clear();

	static auto construct(std::uint32_t count = 1) -> Ref<MaterialTable> { return new MaterialTable(count); }
	static auto construct(const Ref<MaterialTable>& table) -> Ref<MaterialTable> { return new MaterialTable(table); }

private:
	std::unordered_map<std::uint32_t, Ref<MaterialAsset>> materials {};
	std::uint32_t material_count { 0 };
};

} // namespace Disarray
