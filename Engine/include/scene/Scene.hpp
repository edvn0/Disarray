#pragma once

#include <entt/entt.hpp>

#include <concepts>
#include <mutex>
#include <queue>
#include <type_traits>

#include "core/ThreadPool.hpp"
#include "core/Types.hpp"
#include "core/events/Event.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Texture.hpp"
#include "scene/Component.hpp"

namespace Disarray {

enum class GizmoType {
	TranslateX = (1u << 0),
	Translate_Y = (1u << 1),
	TranslateZ = (1u << 2),
	RotateX = (1u << 3),
	RotateY = (1u << 4),
	RotateZ = (1u << 5),
	RotateScreen = (1u << 6),
	ScaleX = (1u << 7),
	ScaleY = (1u << 8),
	ScaleZ = (1u << 9),
	Bounds = (1u << 10),
	Translate = TranslateX | Translate_Y | TranslateZ,
	Rotate = RotateX | RotateY | RotateZ | RotateScreen,
	Scale = ScaleX | ScaleY | ScaleZ
};

class Entity;

class Scene {
public:
	Scene(const Disarray::Device&, std::string_view);
	~Scene();

	void begin_frame(const Camera&);
	void end_frame();

	void update(float);
	void render();
	void construct(Disarray::App&, Disarray::ThreadPool&);
	void destruct();
	void on_event(Disarray::Event&);
	void recreate(const Extent& extent);

	void set_viewport_bounds(const glm::vec2& max, const glm::vec2& min)
	{
		vp_max = max;
		vp_min = min;
	}

	FloatExtent get_viewport_bounds() const { return { vp_max.x - vp_min.x, vp_max.y - vp_min.y }; }

	Entity create(std::string_view = "Unnamed");
	void delete_entity(entt::entity);
	void delete_entity(const Entity& entity);

	Disarray::Image& get_image(std::uint32_t index)
	{
		if (index == 0)
			return identity_framebuffer->get_image(0);
		else if (index == 1)
			return identity_framebuffer->get_image(1);
		else
			return identity_framebuffer->get_depth_image();
	}

	const CommandExecutor& get_command_executor() const { return *command_executor; };

	entt::registry& get_registry() { return registry; };
	const auto& get_selected_entity() const { return selected_entity; }
	const entt::registry& get_registry() const { return registry; };
	const std::string& get_name() const { return scene_name; };

	std::optional<Entity> get_by_identifier(Identifier);

	void update_picked_entity(std::uint32_t handle);
	void manipulate_entity_transform(Entity&, Camera&, GizmoType);

	template <class Func> constexpr void for_all_entities(Func&& func)
	{
		const auto view = get_registry().storage<entt::entity>().each();
		for (const auto& [entity] : view) {
			func(entity);
		}
	}

	static Scope<Scene> deserialise(const Device&, std::string_view, const std::filesystem::path&);

private:
	const Disarray::Device& device;
	std::string scene_name;
	Scope<Renderer> scene_renderer { nullptr };

	Scope<Entity> picked_entity { nullptr };
	Scope<Entity> selected_entity { nullptr };

	Extent extent;
	glm::vec2 vp_max { 1 };
	glm::vec2 vp_min { 1 };

	Ref<Disarray::Framebuffer> framebuffer {};
	Ref<Disarray::Framebuffer> identity_framebuffer {};
	Ref<Disarray::CommandExecutor> command_executor {};

	entt::registry registry;

	using FuncPtr = void (*)(Disarray::Scene&);
	struct ThreadPoolCallback {
		FuncPtr func;
		bool parallel { false };
	};
	std::queue<ThreadPoolCallback> thread_pool_callbacks {};

	std::future<void> final_pool_callback {};
	std::atomic_bool should_run_callbacks { true };
	std::condition_variable callback_cv {};
	std::mutex callback_mutex {};
};

} // namespace Disarray
