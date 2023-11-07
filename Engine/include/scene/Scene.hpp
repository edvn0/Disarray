#pragma once

#include <glm/ext/matrix_clip_space.hpp>

#include <entt/entt.hpp>

#include <concepts>
#include <mutex>
#include <queue>
#include <type_traits>

#include "core/Collections.hpp"
#include "core/FileWatcher.hpp"
#include "core/ThreadPool.hpp"
#include "core/Types.hpp"
#include "core/events/Event.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/StorageBuffer.hpp"
#include "graphics/Texture.hpp"
#include "physics/PhysicsEngine.hpp"
#include "scene/Component.hpp"
#include "scene/Entity.hpp"
#include "scene/SceneRenderer.hpp"

namespace Disarray {

enum class GizmoType : std::uint16_t {
	TranslateX = (1U << 0),
	TranslateY = (1U << 1),
	TranslateZ = (1U << 2),
	RotateX = (1U << 3),
	RotateY = (1U << 4),
	RotateZ = (1U << 5),
	RotateScreen = (1U << 6),
	ScaleX = (1U << 7),
	ScaleY = (1U << 8),
	ScaleZ = (1U << 9),
	Bounds = (1U << 10),
	Translate = TranslateX | TranslateY | TranslateZ,
	Rotate = RotateX | RotateY | RotateZ | RotateScreen,
	Scale = ScaleX | ScaleY | ScaleZ
};

enum class SceneState : std::uint8_t {
	Play,
	Edit,
	Simulate,
};

class Scene : public ReferenceCountable {
	using ViewProjectionTuple = std::tuple<glm::mat4, glm::mat4, glm::mat4>;

public:
	Scene(const Disarray::Device&, std::string_view);
	~Scene() override;

	void begin_frame(const Camera&, SceneRenderer& scene_renderer);
	void begin_frame(const glm::mat4& view, const glm::mat4& proj, const glm::mat4& view_proj, SceneRenderer& scene_renderer);
	void end_frame(SceneRenderer& renderer);

	void update(float);
	void render(SceneRenderer& renderer);
	void interface();
	void construct(Disarray::App&);
	void destruct();
	void on_event(Disarray::Event&);
	void recreate(const Extent&);

	auto get_primary_camera() -> std::optional<ViewProjectionTuple>;

	auto create(std::string_view = "Unnamed") -> Entity;

	template <typename... Args> auto create(fmt::format_string<Args...> format, Args&&... args) -> Entity
	{
		return create(fmt::format(format, std::forward<Args>(args)...));
	}
	void delete_entity(entt::entity);
	void delete_entity(const Entity& entity);

	[[nodiscard]] auto get_selected_entity() const -> const auto& { return selected_entity; }
	auto get_registry() -> entt::registry& { return registry; };
	[[nodiscard]] auto get_registry() const -> const entt::registry& { return registry; };
	[[nodiscard]] auto get_name() const -> const std::string& { return scene_name; };
	[[nodiscard]] auto get_device() const -> const Disarray::Device& { return device; };

	auto on_runtime_start() -> void;
	auto on_simulation_start() -> void;
	auto on_runtime_stop() -> void;
	auto on_simulation_stop() -> void;
	auto on_update_editor(float time_step) -> void;
	auto on_update_simulation(float time_step) -> void;
	auto on_update_runtime(float time_step) -> void;

	template <ValidComponent... T> auto entities_with() -> std::vector<Entity>
	{
		auto view_for = registry.view<T...>();
		std::vector<Entity> out;
		if constexpr (sizeof...(T) == 1) {
			out.reserve(view_for.size());
		} else {
			out.reserve(view_for.size_hint());
		}
		view_for.each([this, &out](auto entity, auto...) { out.push_back(Entity { this, entity }); });
		return out;
	}

	auto get_by_identifier(Identifier) -> std::optional<Entity>;

	template <ValidComponent... Ts> auto get_by_components() -> std::optional<Entity>
	{
		std::vector<Entity> found = entities_with<Ts...>();
		if (found.size() != 1) {
			return std::nullopt;
		}
		return std::optional { found.at(0) };
	}

	void update_picked_entity(std::uint32_t handle);
	void manipulate_entity_transform(Entity&, Camera&, GizmoType);

	constexpr void for_all_entities(auto&& func)
	{
		const auto view = get_registry().storage<entt::entity>().each();
		for (const auto& [entity] : view) {
			func(entity);
		}
	}

	template <ValidComponent T> void sort(auto&& sorter) { registry.sort<T>(sorter); }
	auto sort() -> void;

	static auto deserialise(const Device&, std::string_view, const std::filesystem::path&) -> Scope<Scene>;
	static auto deserialise_into(Scene&, const Device&, const std::filesystem::path&) -> void;

	void clear();

	[[nodiscard]] auto is_paused() const { return paused; }
	auto set_paused(bool new_pause_status) { paused = new_pause_status; }

	void step(std::int32_t steps = 0);

	static auto copy(Scene& scene) -> Ref<Scene>;
	static auto copy_entity(Scene& scene, Entity& entity, std::string_view new_name) -> void;
	static auto copy_entity(Scene& scene, Entity& entity) -> void;

	template <class Func> auto submit_preframe_work(Func&& func) { frame_start_callbacks.emplace(std::forward<Func>(func)); }

private:
	PhysicsEngine engine;
	void physics_update(float time_step);

	const Disarray::Device& device;
	std::string scene_name;

	Scope<Entity> picked_entity { nullptr };
	Scope<Entity> selected_entity { nullptr };

	bool paused { false };
	bool running { false };
	std::int32_t step_frames { 0 };

	Extent extent {};

	entt::registry registry;

	void draw_shadows(SceneRenderer& renderer);
	void draw_identifiers(SceneRenderer& renderer);
	void draw_geometry(SceneRenderer& renderer);
	void draw_skybox(SceneRenderer& renderer);

	auto on_physics_start() -> void;
	auto on_physics_stop() -> void;

	using FrameStartCallback = decltype(+[](Scene&, SceneRenderer&) {});
	std::queue<FrameStartCallback> frame_start_callbacks {};
	void execute_callbacks(SceneRenderer& renderer);

	friend class CppScript;
};

} // namespace Disarray
