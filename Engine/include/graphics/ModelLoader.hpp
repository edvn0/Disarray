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
};

using ImportedSubmesh = Submesh;
using ImportedMesh = Collections::StringMap<ImportedSubmesh>;

struct IModelImporter {
	virtual ~IModelImporter() = default;
	virtual auto import(const std::filesystem::path&) -> ImportedMesh = 0;
};

class ModelLoader {
public:
	explicit ModelLoader(Scope<IModelImporter>);
	explicit ModelLoader(Scope<IModelImporter>, const std::filesystem::path&);
	void import_model(const std::filesystem::path&);
	[[nodiscard]] auto construct_textures(const Device&) -> std::vector<Ref<Disarray::Texture>>;

	[[nodiscard]] auto get_mesh_data() -> const ImportedMesh& { return mesh_data; }
	[[nodiscard]] auto get_aabb() const -> AABB;

private:
	Scope<IModelImporter> importer { nullptr };
	std::filesystem::path mesh_path {};
	ImportedMesh mesh_data {};
};

} // namespace Disarray
