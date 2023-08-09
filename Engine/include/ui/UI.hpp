#pragma once

#include "Forward.hpp"
#include "core/Window.hpp"
#include "graphics/Image.hpp"
#include "graphics/Texture.hpp"

#include <array>
#include <functional>
#include <glm/glm.hpp>

namespace Disarray::UI {

	using ImageIdentifier = std::uint64_t;
	class DescriptorCache {
		using ImageCache = std::unordered_map<ImageIdentifier, std::unique_ptr<ImageIdentifier>>;

	public:
		static void initialise();
		static void destruct();

		static auto& get_cache() { return cache; }

	private:
		inline static ImageCache cache {};
	};

	static constexpr std::array<glm::vec2, 2> default_uvs = { glm::vec2 { 0.f, 0.f }, glm::vec2 { 1.f, 1.f } };

	using UIFunction = std::function<void(void)>;
	static constexpr auto default_function = []() {};

	void image_button(Image&, glm::vec2 size = { 64, 64 }, const std::array<glm::vec2, 2>& uvs = default_uvs);
	void image(Image&, glm::vec2 size = { 64, 64 }, const std::array<glm::vec2, 2>& uvs = default_uvs);

	void scope(std::string_view name, UIFunction&& func = default_function);

	void begin(std::string_view);
	void end();

	bool is_maximised(Window& window);

} // namespace Disarray::UI
