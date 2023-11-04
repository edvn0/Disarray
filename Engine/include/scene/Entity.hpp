#pragma once

#include <entt/entt.hpp>
#include <fmt/core.h>

#include <string>
#include <string_view>

#include "core/Ensure.hpp"
#include "entt/entity/entity.hpp"
#include "scene/Component.hpp"
#include "scene/Components.hpp"

namespace Disarray {

class Scene;

class Entity {
public:
	Entity() = default;
	Entity(Scene*, std::string_view);
	Entity(Scene*, entt::entity, std::string_view);
	Entity(Scene*, entt::entity);
	Entity(Scene* input_scene, entt::id_type input_id)
		: Entity(input_scene, static_cast<entt::entity>(input_id)) {};
	explicit Entity(Scene*);

	static auto deserialise(Scene&, Identifier, std::string_view = "Empty") -> Entity;

	auto get_registry() -> entt::registry&;
	[[nodiscard]] auto get_registry() const -> const entt::registry&;
	[[nodiscard]] auto is_valid() const -> bool { return get_registry().valid(identifier); }
	template <ValidComponent... T> auto has_any() -> decltype(auto) { return get_registry().any_of<T...>(identifier); }
	template <ValidComponent... T> auto has_all() -> decltype(auto) { return get_registry().all_of<T...>(identifier); }
	template <ValidComponent... T> auto has_any() const -> decltype(auto) { return get_registry().any_of<T...>(identifier); }
	template <ValidComponent... T> auto has_all() const -> decltype(auto) { return get_registry().all_of<T...>(identifier); }
	template <ValidComponent... T> auto get_components() -> decltype(auto) { return get_registry().get<T...>(identifier); }
	template <ValidComponent... T> [[nodiscard]] auto get_components() const -> decltype(auto) { return get_registry().get<T...>(identifier); }
	template <ValidComponent T> auto has_component() -> decltype(auto) { return get_registry().any_of<T>(identifier); }
	template <ValidComponent T> [[nodiscard]] auto has_component() const -> decltype(auto) { return get_registry().any_of<T>(identifier); }

	auto get_transform() -> decltype(auto) { return get_components<Components::Transform>(); }

	template <ValidComponent T, typename... Args> auto add_component(Args&&... args) -> decltype(auto)
	{
		if (has_component<T>()) {
			return get_components<T>();
		}
		return emplace_component<T>(std::forward<Args>(args)...);
	}

	template <ValidComponent T, typename... Args> auto put_component(Args&&... args) -> decltype(auto)
	{
		if (has_component<T>()) {
			T constructed { std::forward<Args>(args)... };
			auto& component = get_components<T>();
			component = constructed;
			return component;
		}
		return emplace_component<T>(std::forward<Args>(args)...);
	}

	template <ValidComponent T, typename... Args> auto try_add_component(Args&&... args) -> decltype(auto)
	{
		if (has_component<T>()) {
			return;
		}
		emplace_component<T>(std::forward<Args>(args)...);
	}

	template <ValidComponent T, typename... Args> auto emplace_component(Args&&... args) -> decltype(auto)
	{
		auto& registry = get_registry();
		return registry.emplace<T>(identifier, std::forward<Args>(args)...);
	}

	template <DeletableComponent T> void remove_component()
	{
		if (!has_component<T>()) {
			return;
		}
		auto& registry = get_registry();
		registry.erase<T>(identifier);
	}

	template <class ChildScript, typename... Args>
		requires std::is_base_of_v<CppScript, ChildScript>
	void add_script(Args&&... args)
	{
		ensure(!has_component<Components::Script>(), "Can only attach one script!");
		auto& registry = get_registry();
		auto& script = registry.emplace<Components::Script>(identifier);
		script.bind<ChildScript>(std::forward<Args>(args)...);
	}

	void add_child(Entity&);
	void add_child(Entity* = nullptr);

	auto operator==(const Entity& other) const { return identifier == other.identifier; }
	auto operator!=(const Entity& other) const { return !operator==(other); }

	[[nodiscard]] auto get_identifier() const -> const auto& { return identifier; }

private:
	Scene* scene { nullptr };
	std::string name {};
	entt::entity identifier { entt::null };

	friend class CppScript;
};

class ImmutableEntity {
public:
	ImmutableEntity(const Scene*, entt::entity, std::string_view);
	ImmutableEntity(const Scene*, entt::entity);
	ImmutableEntity(const Scene* input_scene, entt::id_type input_id)
		: ImmutableEntity(input_scene, static_cast<entt::entity>(input_id)) {};
	ImmutableEntity(const Scene*);

	[[nodiscard]] auto get_registry() const -> const entt::registry&;

	[[nodiscard]] auto is_valid() const -> bool { return get_registry().valid(identifier); }
	template <ValidComponent... T> auto has_any() const -> decltype(auto) { return get_registry().any_of<T...>(identifier); }
	template <ValidComponent... T> auto has_all() const -> decltype(auto) { return get_registry().all_of<T...>(identifier); }
	template <ValidComponent... T> auto get_components() const -> decltype(auto) { return get_registry().get<T...>(identifier); }
	template <ValidComponent T> auto has_component() const -> decltype(auto) { return get_registry().any_of<T>(identifier); }

	auto operator==(const ImmutableEntity& other) const { return identifier == other.identifier; }
	auto operator!=(const ImmutableEntity& other) const { return !operator==(other); }

	[[nodiscard]] auto get_identifier() const -> const auto& { return identifier; }

private:
	const Scene* scene { nullptr };
	std::string name {};
	entt::entity identifier { entt::null };

	friend class CppScript;
};

} // namespace Disarray

template <> struct fmt::formatter<entt::entity> : fmt::formatter<std::string_view> {
	auto format(entt::entity entity, fmt::format_context& ctx) const -> decltype(ctx.out());
};

template <> struct fmt::formatter<Disarray::Entity> : fmt::formatter<std::string_view> {
	auto format(const Disarray::Entity& entity, fmt::format_context& ctx) const -> decltype(ctx.out());
};
