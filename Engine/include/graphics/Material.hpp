#pragma once

#include <utility>

#include "Forward.hpp"
#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "graphics/Shader.hpp"

namespace Disarray {

struct MaterialProperties {
	Ref<Shader> vertex_shader;
	Ref<Shader> fragment_shader;
	std::vector<Ref<Texture>> textures {};
};

class Material : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(Material, MaterialProperties)
public:
	virtual void update_material(Renderer&) = 0;
};

} // namespace Disarray
