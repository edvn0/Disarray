#pragma once

#include "core/ThreadPool.hpp"
#include "core/Types.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Texture.hpp"

#include <entt/entt.hpp>

namespace Disarray {

	class Entity;

	class Scene {
	public:
		Scene(Disarray::Device&, Disarray::Window&, Disarray::Swapchain&, std::string_view);
		~Scene();
		void update(float);
		void render(float, Disarray::Renderer&);
		void construct(Disarray::App&, Disarray::Renderer&, Disarray::ThreadPool&);
		void destruct();
		void recreate(const Extent& extent);

		Entity create(std::string_view = "Unnamed");

		Disarray::Framebuffer& get_framebuffer();
		const CommandExecutor& get_command_executor() const { return *command_executor; };

		entt::registry& get_registry() { return registry; };

	private:
		Disarray::Device& device;
		Disarray::Window& window;
		Disarray::Swapchain& swapchain;
		std::string scene_name;

		Ref<Disarray::Framebuffer> framebuffer;
		Ref<Disarray::Framebuffer> identity_framebuffer;
		Ref<Disarray::CommandExecutor> command_executor;

		Ref<Disarray::Pipeline> pipeline;
		Ref<Disarray::Pipeline> geometry_pipeline;

		Ref<Disarray::Mesh> viking_mesh;
		Ref<Disarray::Texture> viking_room;

		// Should contain some kind of container for entities :)
		entt::registry registry;
	};

} // namespace Disarray