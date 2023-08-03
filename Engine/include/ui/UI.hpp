#pragma once

#include "core/Window.hpp"
#include "graphics/Image.hpp"
#include "graphics/Texture.hpp"

#include <array>
#include <functional>
#include <glm/glm.hpp>

namespace Disarray::UI {

	static constexpr std::array<glm::vec2, 2> default_uvs = { glm::vec2 { 0.f, 0.f }, glm::vec2 { 1.f, 1.f } };

	using UIFunction = std::function<void(void)>;
	static constexpr auto default_function = []() {};

	void image_button(Image&, glm::vec2 size = { 64, 64 }, const std::array<glm::vec2, 2>& uvs = default_uvs);
	void image(Image&, glm::vec2 size = { 64, 64 }, const std::array<glm::vec2, 2>& uvs = default_uvs);

	void scope(std::string_view name, UIFunction&& func = default_function);

	bool is_maximised(Window& window);

} // namespace Disarray::UI
