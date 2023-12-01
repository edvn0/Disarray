#pragma once

#include "Forward.hpp"

#include "core/Collections.hpp"
#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "graphics/Shader.hpp"
#include "graphics/Swapchain.hpp"
#include "graphics/Texture.hpp"
#include "graphics/UnifiedShader.hpp"

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
	Ref<UnifiedShader> shader;
	std::uint32_t swapchain_image_count;
	std::string name;

	explicit POCMaterialProperties(Ref<UnifiedShader> input_shader, std::string input_name = "Empty", std::uint32_t input_image_count = 3)
		: shader { std::move(input_shader) }
		, swapchain_image_count { input_image_count }
		, name { std::move(input_name) }
	{
		if (name == "Empty") {
			name = shader->get_name();
		}
	}
};

class POCMaterial : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(POCMaterial, POCMaterialProperties)
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
	virtual void set(const std::string&, const Ref<Texture>&) = 0;
	virtual void set(const std::string&, const Ref<Texture>&, std::uint32_t) = 0;
	virtual void set(const std::string&, const Ref<Image>& image) = 0;
};

} // namespace Disarray
