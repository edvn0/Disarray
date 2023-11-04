#pragma once

#include <entt/entt.hpp>

#include <concepts>
#include <mutex>
#include <queue>
#include <type_traits>

#include "SceneRenderer.hpp"
#include "core/Collections.hpp"
#include "core/FileWatcher.hpp"
#include "core/ThreadPool.hpp"
#include "core/Types.hpp"
#include "core/events/Event.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/StorageBuffer.hpp"
#include "graphics/Texture.hpp"
#include "scene/Component.hpp"
#include "scene/Entity.hpp"

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

class Scene {
public:
	Scene(const Disarray::Device&, std::string_view);
	~Scene();

	void begin_frame(const Camera&, SceneRenderer& scene_renderer);
	void end_frame(SceneRenderer& renderer);

	void update(float);
	void render(SceneRenderer& renderer);
	void interface();
	void construct(Disarray::App&);
	void destruct();
	void on_event(Disarray::Event&);
	void recreate(const Extent&);

	auto create(std::string_view = "Unnamed") -> Entity;

	template <typename... Args> auto create(fmt::format_string<Args...> format, Args&&... args) -> Entity
	{
		return create(fmt::format(format, std::forward<Args>(args)...));
	}
	void delete_entity(entt::entity);
	void delete_entity(const Entity& entity);

	auto get_selected_entity() const -> const auto& { return selected_entity; }
	auto get_registry() -> entt::registry& { return registry; };
	auto get_registry() const -> const entt::registry& { return registry; };
	auto get_name() const -> const std::string& { return scene_name; };
	auto get_device() const -> const Disarray::Device& { return device; };

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

	template <class Func> constexpr void for_all_entities(Func&& func)
	{
		const auto view = get_registry().storage<entt::entity>().each();
		for (const auto& [entity] : view) {
			std::forward<Func>(func)(entity);
		}
	}

	static auto deserialise(const Device&, std::string_view, const std::filesystem::path&) -> Scope<Scene>;
	static auto deserialise_into(Scene&, const Device&, const std::filesystem::path&) -> void;

	void clear();

	static auto copy(Scene& scene) -> Scope<Scene>;

private:
	const Disarray::Device& device;
	std::string scene_name;

	Scope<Entity> picked_entity { nullptr };
	Scope<Entity> selected_entity { nullptr };

	Extent extent {};

	entt::registry registry;

	void draw_shadows(SceneRenderer& renderer);
	void draw_identifiers(SceneRenderer& renderer);
	void draw_geometry(SceneRenderer& renderer);
	void draw_skybox(SceneRenderer& renderer);

	friend class CppScript;
};

} // namespace Disarray
