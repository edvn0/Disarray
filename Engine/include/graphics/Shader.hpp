#pragma once

#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/Types.hpp"
#include "graphics/Device.hpp"

#include <filesystem>
#include <string_view>

namespace Disarray {

	enum class ShaderType { Vertex, Fragment, Compute };
	static constexpr auto shader_type_extension(ShaderType shader_type)
	{
		switch (shader_type) {
		case ShaderType::Vertex:
			return ".vert";
		case ShaderType::Fragment:
			return ".frag";
		case ShaderType::Compute:
			return ".comp";
		default:
			unreachable();
		}
	}

	struct ShaderProperties {
		std::filesystem::path path;
		ShaderType type { ShaderType::Vertex };
		std::string entry_point = "main";
	};

	class Shader : public ReferenceCountable {
		DISARRAY_OBJECT(Shader)
	public:
		virtual void destroy_module() = 0;
		virtual const ShaderProperties& get_properties() const = 0;
		virtual ShaderProperties& get_properties() = 0;

		static Ref<Disarray::Shader> construct(const Disarray::Device& device, const ShaderProperties&);
	};

} // namespace Disarray
