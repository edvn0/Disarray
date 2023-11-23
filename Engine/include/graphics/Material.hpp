#pragma once

#include "Forward.hpp"

#include "core/Collections.hpp"
#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "graphics/Shader.hpp"
#include "graphics/Swapchain.hpp"
#include "graphics/Texture.hpp"

namespace Disarray {

struct MaterialProperties {
	Collections::ReferencedStringMap<Disarray::Texture> textures {};
};

class Material : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(Material, MaterialProperties)
public:
	virtual void update_material(Renderer&) = 0;
	virtual void write_textures(IGraphicsResource& resource) const = 0;

	virtual void bind(const Disarray::CommandExecutor&, const Disarray::Pipeline&, FrameIndex) const = 0;
};

struct POCMaterialProperties {
	Ref<Disarray::Shader> vertex;
	Ref<Disarray::Shader> fragment;
	std::string name;
};

class POCMaterial : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(POCMaterial, POCMaterialProperties)
};

} // namespace Disarray
