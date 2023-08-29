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

enum class GizmoType : std::uint16_t {
	TranslateX = (1U << 0),
	Translate_Y = (1U << 1),
	TranslateZ = (1U << 2),
	RotateX = (1U << 3),
	RotateY = (1U << 4),
	RotateZ = (1U << 5),
	RotateScreen = (1U << 6),
	ScaleX = (1U << 7),
	ScaleY = (1U << 8),
	ScaleZ = (1U << 9),
	Bounds = (1U << 10),
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
	void interface();
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

	auto create(std::string_view = "Unnamed") -> Entity;

	template <typename... Args> auto create(fmt::format_string<Args...> format, Args&&... args) -> Entity
	{
		return create(fmt::format(format, std::forward<Args>(args)...));
	}
	void delete_entity(entt::entity);
	void delete_entity(const Entity& entity);

	auto get_image(std::uint32_t index) -> Disarray::Image&
	{
		if (index == 0) {
			return identity_framebuffer->get_image(0);
		} else if (index == 1)
			return identity_framebuffer->get_image(1);
		else
			return identity_framebuffer->get_depth_image();
	}

	auto get_command_executor() const -> const CommandExecutor& { return *command_executor; };

	auto get_selected_entity() const -> const auto& { return selected_entity; }
	auto get_registry() -> entt::registry& { return registry; };
	auto get_registry() const -> const entt::registry& { return registry; };
	auto get_name() const -> const std::string& { return scene_name; };

	template <ValidComponent... T> auto entities_with() -> std::vector<Entity>
	{
		auto view_for = registry.view<T...>();
		std::vector<Entity> out;
		if constexpr (sizeof...(T) == 1)
			out.reserve(view_for.size());
		else
			out.reserve(view_for.size_hint());
		view_for.each([this, &out](auto entity, auto... ts) { out.push_back(Entity { this, entity }); });
		return out;
	}

	auto get_by_identifier(Identifier) -> std::optional<Entity>;

	void update_picked_entity(std::uint32_t handle);
	static void manipulate_entity_transform(Entity&, Camera&, GizmoType);

	template <class Func> constexpr void for_all_entities(Func&& func)
	{
		const auto view = get_registry().storage<entt::entity>().each();
		for (const auto& [entity] : view) {
			std::forward<Func>(func)(entity);
		}
	}

	static auto deserialise(const Device&, std::string_view, const std::filesystem::path&) -> Scope<Scene>;

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

	using FuncPtr = void (*)(const Disarray::Scene*);
	struct ThreadPoolCallback {
		FuncPtr func { nullptr };
		bool parallel { false };
	};
	std::queue<ThreadPoolCallback> thread_pool_callbacks {};

	std::future<void> final_pool_callback {};
	std::atomic_bool should_run_callbacks { true };
	std::condition_variable callback_cv {};
	std::mutex callback_mutex {};

	friend class CppScript;
};

} // namespace Disarray
