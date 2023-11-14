#pragma once

#include "Forward.hpp"

#include "core/Collections.hpp"
#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "graphics/Texture.hpp"

namespace Disarray {

struct MaterialProperties {
	Collections::ReferencedStringMap<Disarray::Texture> textures {};
};

class Material : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(Material, MaterialProperties)
public:
	virtual void update_material(Renderer&) = 0;
};

} // namespace Disarray
