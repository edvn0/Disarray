#pragma once

#include <entt/entt.hpp>
#include <fmt/core.h>

#include <string>
#include <string_view>

#include "core/Ensure.hpp"
#include "scene/Component.hpp"

namespace Disarray {

class Scene;

class Entity {
public:
	Entity()
		: scene(nullptr)
	{
	}
	Entity(Scene*, std::string_view);
	Entity(Scene*, entt::entity, std::string_view);
	Entity(Scene*, entt::entity);
	Entity(Scene* s, entt::id_type id)
		: Entity(s, static_cast<entt::entity>(id)) {};
	Entity(Scene*);

	static Entity deserialise(Scene&, entt::entity, Identifier, std::string_view = "Empty");

	auto get_registry() -> entt::registry&;
	auto get_registry() const -> const entt::registry&;

	bool is_valid() const { return get_registry().valid(identifier); }
	template <ValidComponent... T> decltype(auto) has_any() { return get_registry().any_of<T...>(identifier); }
	template <ValidComponent... T> decltype(auto) has_all() { return get_registry().all_of<T...>(identifier); }
	template <ValidComponent... T> decltype(auto) has_any() const { return get_registry().any_of<T...>(identifier); }
	template <ValidComponent... T> decltype(auto) has_all() const { return get_registry().all_of<T...>(identifier); }
	template <ValidComponent... T> decltype(auto) get_components() { return get_registry().get<T...>(identifier); }
	template <ValidComponent... T> decltype(auto) get_components() const { return get_registry().get<T...>(identifier); }
	template <ValidComponent T> decltype(auto) has_component() { return get_registry().any_of<T>(identifier); }
	template <ValidComponent T> decltype(auto) has_component() const { return get_registry().any_of<T>(identifier); }

	template <ValidComponent T, typename... Args> decltype(auto) add_component(Args&&... args)
	{
		if (has_component<T>())
			return get_components<T>();
		return emplace_component<T>(std::forward<Args>(args)...);
	}

	template <ValidComponent T, typename... Args> decltype(auto) try_add_component(Args&&... args)
	{
		if (has_component<T>())
			return;
		emplace_component<T>(std::forward<Args>(args)...);
	}

	template <ValidComponent T, typename... Args> decltype(auto) emplace_component(Args&&... args)
	{
		auto& registry = get_registry();
		return registry.emplace<T>(identifier, std::forward<Args>(args)...);
	}

	template <DeletableComponent T> void remove_component()
	{
		if (!has_component<T>())
			return;
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

	auto get_identifier() const -> const auto& { return identifier; }

private:
	Scene* scene;
	std::string name;
	entt::entity identifier;

	friend class CppScript;
};

class ImmutableEntity {
public:
	ImmutableEntity(const Scene*, entt::entity, std::string_view);
	ImmutableEntity(const Scene*, entt::entity);
	ImmutableEntity(const Scene* s, entt::id_type id)
		: ImmutableEntity(s, static_cast<entt::entity>(id)) {};
	ImmutableEntity(const Scene*);

	auto get_registry() const -> const entt::registry&;

	bool is_valid() const { return get_registry().valid(identifier); }
	template <ValidComponent... T> decltype(auto) has_any() const { return get_registry().any_of<T...>(identifier); }
	template <ValidComponent... T> decltype(auto) has_all() const { return get_registry().all_of<T...>(identifier); }
	template <ValidComponent... T> decltype(auto) get_components() const { return get_registry().get<T...>(identifier); }
	template <ValidComponent T> decltype(auto) has_component() const { return get_registry().any_of<T>(identifier); }

	auto operator==(const ImmutableEntity& other) const { return identifier == other.identifier; }
	auto operator!=(const ImmutableEntity& other) const { return !operator==(other); }

	auto get_identifier() const -> const auto& { return identifier; }

private:
	const Scene* scene;
	std::string name;
	entt::entity identifier;

	friend class CppScript;
};

} // namespace Disarray

template <> struct fmt::formatter<entt::entity> : fmt::formatter<std::string_view> {
	auto format(entt::entity entity, fmt::format_context& ctx) const -> decltype(ctx.out());
};

template <> struct fmt::formatter<Disarray::Entity> : fmt::formatter<std::string_view> {
	auto format(const Disarray::Entity& entity, fmt::format_context& ctx) const -> decltype(ctx.out());
};
