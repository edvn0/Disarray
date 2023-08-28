#include "DisarrayPCH.hpp"

#include "scene/Components.hpp"

#include "core/Log.hpp"
#include "graphics/Mesh.hpp"
#include "scene/CppScript.hpp"
#include "scene/Entity.hpp"
#include "scene/Scene.hpp"

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

Material::Material(Device& device, std::string_view vertex, std::string_view fragment)
	: material(Disarray::Material::construct(device,
		MaterialProperties {
			.vertex_shader = Shader::construct(device,
				{
					.path = std::filesystem::path { vertex },
				}),
			.fragment_shader = Shader::construct(device,
				{
					.path = std::filesystem::path { fragment },
				}),
		}))
{
}

Material::Material(Ref<Disarray::Material> m)
	: material(m)
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

void Script::setup_entity_destruction()
{
	destroy_script_functor = [](Script& script) {
		script.instance_slot->on_destroy();
		delete script.instance_slot;
		script.instance_slot = nullptr;
	};
}

void Script::setup_entity_creation() { get_script().on_create(); }

} // namespace Disarray::Components
