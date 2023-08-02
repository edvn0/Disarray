#pragma once

#include "graphics/Image.hpp"
#include "graphics/Texture.hpp"

#include <glm/glm.hpp>

namespace Disarray::UI {

	void image_button(Image&, glm::vec2 size = { 64, 64 });
	void image(Image&, glm::vec2 size = { 64, 64 });

} // namespace Disarray::UI
