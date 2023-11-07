#pragma once

#include "core/Collections.hpp"
#include "core/DisarrayObject.hpp"
#include "core/Types.hpp"
#include "graphics/AABB.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/ModelLoader.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/Texture.hpp"
#include "graphics/VertexBuffer.hpp"

namespace Disarray::Vulkan {

class Mesh : public Disarray::Mesh {
	DISARRAY_MAKE_NONCOPYABLE(Mesh)
public:
	Mesh(const Disarray::Device&, MeshProperties);
	Mesh(const Disarray::Device&, const Submesh&);
	~Mesh() override;

	auto get_indices() const -> Disarray::IndexBuffer& override;
	auto get_vertices() const -> Disarray::VertexBuffer& override;
	auto get_aabb() const -> const AABB& override;

	[[nodiscard]] auto invalid() const -> bool override;

	auto get_submeshes() const -> const Collections::ScopedStringMap<Disarray::MeshSubstructure>& override;
	auto get_textures() const -> const RefVector<Disarray::Texture>& override;
	auto has_children() const -> bool override;

	void force_recreation() override;

	static auto construct_deferred(const Disarray::Device&, MeshProperties) -> std::future<Ref<Disarray::Mesh>>;

private:
	void load_and_initialise_model();

	const Disarray::Device& device;

	std::vector<Ref<Disarray::Texture>> mesh_textures;
	Collections::ScopedStringMap<Disarray::MeshSubstructure> submeshes {};
	AABB aabb {};
	std::string mesh_name {};
};

} // namespace Disarray::Vulkan
