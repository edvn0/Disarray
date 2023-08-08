#pragma once

#include "core/ThreadPool.hpp"
#include "core/Types.hpp"
#include "core/events/Event.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Texture.hpp"
#include "scene/Component.hpp"

#include <entt/entt.hpp>

namespace Disarray {

	class Entity;

	class Scene {
	public:
		Scene(Disarray::Device&, Disarray::Window&, Disarray::Swapchain&, std::string_view);
		~Scene();
		void update(float);
		void render(Disarray::Renderer&);
		void construct(Disarray::App&, Disarray::Renderer&, Disarray::ThreadPool&);
		void destruct();
		void on_event(Disarray::Event&);
		void recreate(const Extent& extent);

		void set_viewport_bounds(const glm::vec2& max, const glm::vec2& min)
		{
			vp_max = max;
			vp_min = min;
		}

		Entity create(std::string_view = "Unnamed");

		Disarray::Image& get_image(std::uint32_t index)
		{
			if (index == 0)
				return framebuffer->get_image();
			else if (index == 1)
				return identity_framebuffer->get_image(1);
			else
				return identity_framebuffer->get_depth_image();
		}

		const CommandExecutor& get_command_executor() const { return *command_executor; };

		entt::registry& get_registry() { return registry; };

	private:
		Disarray::Device& device;
		Disarray::Window& window;
		Disarray::Swapchain& swapchain;
		std::string scene_name;

		glm::vec2 vp_max { 1 };
		glm::vec2 vp_min { 1 };

		Ref<Disarray::Framebuffer> framebuffer;
		Ref<Disarray::Framebuffer> identity_framebuffer;
		Ref<Disarray::CommandExecutor> command_executor;

		// Should contain some kind of container for entities :)
		entt::registry registry;
	};

} // namespace Disarray
