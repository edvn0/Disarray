#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "core/Collections.hpp"
#include "core/PointerDefinition.hpp"
#include "graphics/AABB.hpp"
#include "graphics/ModelVertex.hpp"
#include "graphics/Texture.hpp"

namespace Disarray {

template <class T>
concept SubmeshMember = AnyOf<T, ModelVertex, std::uint32_t, Disarray::TextureProperties>;

struct Submesh {
	std::vector<ModelVertex> vertices {};
	std::vector<uint32_t> indices {};
	std::vector<Disarray::TextureProperties> texture_properties {};
	std::vector<Ref<Disarray::Texture>> textures {};

	template <SubmeshMember T> [[nodiscard]] auto count() const -> std::size_t
	{
		if constexpr (std::is_same_v<T, ModelVertex>) {
			return vertices.size();
		}

		if constexpr (std::is_same_v<T, std::uint32_t>) {
			return indices.size();
		}

		if constexpr (std::is_same_v<T, Disarray::TextureProperties>) {
			return texture_properties.size();
		}
	}

	template <SubmeshMember T> [[nodiscard]] auto size() const -> std::size_t { return count<T>() * sizeof(T); }

	template <SubmeshMember T> [[nodiscard]] auto data() const -> const T*
	{
		if constexpr (std::is_same_v<T, ModelVertex>) {
			return vertices.data();
		}

		if constexpr (std::is_same_v<T, std::uint32_t>) {
			return indices.data();
		}

		if constexpr (std::is_same_v<T, Disarray::TextureProperties>) {
			return texture_properties.data();
		}
	}

	template <SubmeshMember T, class Mapper> [[nodiscard]] auto map_data(Mapper&& mapper) const -> decltype(auto)
	{
		if constexpr (std::is_same_v<T, ModelVertex>) {
			return Collections::map(vertices, std::forward<Mapper>(mapper));
		}

		if constexpr (std::is_same_v<T, std::uint32_t>) {
			return Collections::map(indices, std::forward<Mapper>(mapper));
		}

		if constexpr (std::is_same_v<T, Disarray::TextureProperties>) {
			return Collections::map(texture_properties, std::forward<Mapper>(mapper));
		}
	}
};

using ImportedSubmesh = Submesh;
using ImportedMesh = Collections::StringMap<ImportedSubmesh>;

enum class ImportFlag : std::uint32_t {
	CalcTangentSpace = 0x1,
	JoinIdenticalVertices = 0x2,
	MakeLeftHanded = 0x4,
	Triangulate = 0x8,
	RemoveComponent = 0x10,
	GenNormals = 0x20,
	GenSmoothNormals = 0x40,
	SplitLargeMeshes = 0x80,
	PreTransformVertices = 0x100,
	LimitBoneWeights = 0x200,
	ValidateDataStructure = 0x400,
	ImproveCacheLocality = 0x800,
	RemoveRedundantMaterials = 0x1000,
	FixInfacingNormals = 0x2000,
	PopulateArmatureData = 0x4000,
	SortByPType = 0x8000,
	FindDegenerates = 0x10000,
	FindInvalidData = 0x20000,
	GenUVCoords = 0x40000,
	TransformUVCoords = 0x80000,
	FindInstances = 0x100000,
	OptimizeMeshes = 0x200000,
	OptimizeGraph = 0x400000,
	FlipUVs = 0x800000,
	FlipWindingOrder = 0x1000000,
	SplitByBoneCount = 0x2000000,
	Debone = 0x4000000,
	GlobalScale = 0x8000000,
	EmbedTextures = 0x10000000,
	ForceGenNormals = 0x20000000,
	DropNormals = 0x40000000,
	GenBoundingBoxes = 0x80000000,
};
static constexpr auto operator|(const ImportFlag& left, const ImportFlag& right)
{
	return static_cast<ImportFlag>(static_cast<std::uint32_t>(left) | static_cast<std::uint32_t>(right));
}
static constexpr auto operator&(const ImportFlag& left, const ImportFlag& right)
{
	return static_cast<ImportFlag>(static_cast<std::uint32_t>(left) | static_cast<std::uint32_t>(right));
}
static constexpr auto default_import_flags = ImportFlag::OptimizeMeshes | ImportFlag::OptimizeGraph | ImportFlag::CalcTangentSpace
	| ImportFlag::RemoveRedundantMaterials | ImportFlag::SplitLargeMeshes | ImportFlag::Triangulate | ImportFlag::FlipUVs;

struct IModelImporter {
	virtual ~IModelImporter() = default;
	virtual auto import(const std::filesystem::path&, ImportFlag) -> ImportedMesh = 0;
};

class ModelLoader {
public:
	explicit ModelLoader() = default;
	explicit ModelLoader(Scope<IModelImporter>);
	explicit ModelLoader(Scope<IModelImporter>, const std::filesystem::path&, ImportFlag);
	void import_model(const std::filesystem::path&, ImportFlag);
	[[nodiscard]] auto construct_textures(const Device&) -> std::vector<Ref<Disarray::Texture>>;

	[[nodiscard]] auto get_mesh_data() -> const ImportedMesh& { return mesh_data; }
	[[nodiscard]] auto get_aabb() const -> AABB;

private:
	Scope<IModelImporter> importer { nullptr };
	std::filesystem::path mesh_path {};
	ImportedMesh mesh_data {};
};

} // namespace Disarray
