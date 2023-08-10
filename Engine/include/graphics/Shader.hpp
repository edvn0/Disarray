#pragma once

#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/Types.hpp"
#include "graphics/Device.hpp"

#include <filesystem>
#include <string_view>

namespace Disarray {

	enum class ShaderType { Vertex, Fragment, Compute };

	struct ShaderProperties {
		std::filesystem::path path;
		ShaderType type { ShaderType::Vertex };
		std::string_view entry_point = "main";
	};

	class Shader : public ReferenceCountable {
		DISARRAY_OBJECT(Shader)
	public:
		virtual std::string_view path() const = 0;
		virtual void destroy_module() = 0;

		static Ref<Disarray::Shader> construct(Disarray::Device& device, const ShaderProperties&);
	};

} // namespace Disarray
