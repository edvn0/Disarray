#pragma once

#include "core/Types.hpp"
#include <filesystem>
#include <string_view>

namespace Disarray {

	class Device;

	enum class ShaderType {
		Vertex,
		Fragment
	};

	struct ShaderProperties {
		std::filesystem::path path;
		ShaderType type { ShaderType::Vertex };
		std::string_view entry_point = "main";
	};

	class Shader {
	public:
		virtual ~Shader() = default;
		virtual std::string_view path() const = 0;
		virtual void destroy_module() = 0;

		static Ref<Shader> construct(Ref<Device> device, const ShaderProperties&);
	};

}