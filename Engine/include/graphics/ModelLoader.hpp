#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "core/Collections.hpp"
#include "core/PointerDefinition.hpp"
#include "graphics/ModelVertex.hpp"
#include "graphics/Texture.hpp"

namespace Disarray {

struct Submesh {
	std::vector<ModelVertex> vertices {};
	std::vector<uint32_t> indices {};
	std::vector<Disarray::TextureProperties> texture_properties {};
	std::vector<Ref<Disarray::Texture>> textures {};

	template <class T>
		requires(AnyOf<T, ModelVertex, std::uint32_t, Disarray::TextureProperties>)
	[[nodiscard]] auto count() const -> std::size_t
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

	template <class T>
		requires(AnyOf<T, ModelVertex, std::uint32_t, Disarray::TextureProperties>)
	[[nodiscard]] auto size() const -> std::size_t
	{
		return count<T>() * sizeof(T);
	}

	template <class T>
		requires(AnyOf<T, ModelVertex, std::uint32_t, Disarray::TextureProperties>)
	[[nodiscard]] auto data() const -> const T*
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
	void construct_textures(const Device&);

	[[nodiscard]] auto get_mesh_data() -> const ImportedMesh& { return mesh_data; }

private:
	Scope<IModelImporter> importer { nullptr };
	ImportedMesh mesh_data {};
};

} // namespace Disarray
