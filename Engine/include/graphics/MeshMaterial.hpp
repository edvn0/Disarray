#pragma once

#include <glm/glm.hpp>

#include <cstdint>
#include <string>

#include "core/DataBuffer.hpp"
#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "graphics/Image.hpp"
#include "graphics/SingleShader.hpp"
#include "graphics/Texture.hpp"

namespace Disarray {

struct MeshMaterialProperties {
	Ref<SingleShader> shader { nullptr };
	std::uint32_t swapchain_image_count { 0 };
	std::string name {};

	MeshMaterialProperties() = default;
	explicit MeshMaterialProperties(Ref<SingleShader> input_shader, std::string input_name = "Empty", std::uint32_t input_image_count = 3)
		: shader { std::move(input_shader) }
		, swapchain_image_count { input_image_count }
		, name { std::move(input_name) }
	{
		if (name == "Empty") {
			name = shader->get_name();
		}
	}
};

class MeshMaterial : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(MeshMaterial, MeshMaterialProperties);

public:
	virtual auto get_uniform_storage_buffer() const -> const DataBuffer& = 0;

	virtual void set(const std::string&, float) = 0;
	virtual void set(const std::string&, int) = 0;
	virtual void set(const std::string&, std::uint32_t) = 0;
	virtual void set(const std::string&, bool) = 0;
	virtual void set(const std::string&, const glm::ivec2&) = 0;
	virtual void set(const std::string&, const glm::ivec3&) = 0;
	virtual void set(const std::string&, const glm::ivec4&) = 0;
	virtual void set(const std::string&, const glm::uvec2&) = 0;
	virtual void set(const std::string&, const glm::uvec3&) = 0;
	virtual void set(const std::string&, const glm::uvec4&) = 0;
	virtual void set(const std::string&, const glm::vec2&) = 0;
	virtual void set(const std::string&, const glm::vec3&) = 0;
	virtual void set(const std::string&, const glm::vec4&) = 0;
	virtual void set(const std::string&, const glm::mat3&) = 0;
	virtual void set(const std::string&, const glm::mat4&) = 0;
	virtual void set(const std::string&, const Ref<Disarray::Texture>&) = 0;
	virtual void set(const std::string&, const Ref<Disarray::Texture>&, std::uint32_t) = 0;
	virtual void set(const std::string&, const Ref<Disarray::Image>& image) = 0;
};

} // namespace Disarray
