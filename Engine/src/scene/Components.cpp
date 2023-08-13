#include "DisarrayPCH.hpp"

#include "graphics/Mesh.hpp"
#include "scene/Components.hpp"
#include "scene/Entity.hpp"

namespace Disarray::Components {

void Inheritance::add_child(Entity& entity) { children.insert(entity.get_components<Components::ID>().identifier); }

Mesh::Mesh(Device& device, std::string_view path)
	: mesh(Disarray::Mesh::construct(device, MeshProperties { .path = std::string { path } }))
{
}

Mesh::Mesh(Ref<Disarray::Mesh> m)
	: mesh(m)
{
}

Pipeline::Pipeline(Ref<Disarray::Pipeline> p)
	: pipeline(p)
{
}

Texture::Texture(Device& device, std::string_view path)
	: texture(Disarray::Texture::construct(device, TextureProperties { .path = std::string { path } }))
{
}

Texture::Texture(Ref<Disarray::Texture> m, const glm::vec4& colour)
	: texture(m)
	, colour(colour)
{
}

Texture::Texture(const glm::vec4& colour)
	: colour(colour)
{
}

} // namespace Disarray::Components
