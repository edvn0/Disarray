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
};

class Material : public ReferenceCountable {
	DISARRAY_OBJECT(Material)
public:
	static auto construct(const Disarray::Device&, const MaterialProperties&) -> Ref<Material>;
	auto get_properties() const -> const MaterialProperties& { return props; };

	virtual void update_material(Renderer&) = 0;

protected:
	Material(MaterialProperties properties)
		: props(std::move(properties))
	{
	}

private:
	MaterialProperties props;
};

} // namespace Disarray
