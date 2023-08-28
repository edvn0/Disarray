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

void Script::delete_script(ScriptPtr& script)
{
	Log::info("Script Component", "Deleted script pointer: {}", static_cast<const void*>(script.get()));
	if (script != nullptr) {
		script->on_destroy();
		script.reset();
	}
}

void Script::create_script(ScriptPtr& script)
{
	auto name = script->identifier();
	Log::info("Script Component", "Created script pointer with name {}: {}", name, static_cast<const void*>(script.get()));
	if (script != nullptr) {
		script->on_create();
	}
}

Script::~Script() { Log::info("Script Component", "Destructor for {} called.", static_cast<const void*>(this)); }

} // namespace Disarray::Components
