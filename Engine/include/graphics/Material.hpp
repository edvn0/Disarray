#pragma once

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
		static Ref<Material> construct(const Disarray::Device&, const MaterialProperties&);
		const MaterialProperties& get_properties() const { return props; };

		virtual void update_material(Renderer&) = 0;

	protected:
		Material(const MaterialProperties& properties)
			: props(properties)
		{
		}

	private:
		MaterialProperties props;
	};

} // namespace Disarray
