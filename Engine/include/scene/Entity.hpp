#pragma once

#include "scene/Component.hpp"
#include "scene/Scene.hpp"

#include <entt/entt.hpp>
#include <fmt/core.h>
#include <string>
#include <string_view>

namespace Disarray {

	class Entity {
	public:
		Entity(Scene&, std::string_view = "Empty");
		Entity(Scene&, entt::entity, std::string_view = "Empty");

		static Entity deserialise(Scene&, entt::entity, Identifier, std::string_view = "Empty");

		template <ValidComponent... T> decltype(auto) has_any() { return scene.get_registry().any_of<T...>(identifier); }
		template <ValidComponent... T> decltype(auto) has_all() { return scene.get_registry().all_of<T...>(identifier); }
		template <ValidComponent... T> decltype(auto) has_any() const { return scene.get_registry().any_of<T...>(identifier); }
		template <ValidComponent... T> decltype(auto) has_all() const { return scene.get_registry().all_of<T...>(identifier); }
		template <ValidComponent... T> decltype(auto) get_components() { return scene.get_registry().get<T...>(identifier); }
		template <ValidComponent... T> decltype(auto) get_components() const { return scene.get_registry().get<T...>(identifier); }
		template <ValidComponent T> decltype(auto) has_component() { return scene.get_registry().any_of<T>(identifier); }
		template <ValidComponent T> decltype(auto) has_component() const { return scene.get_registry().any_of<T>(identifier); }

		template <ValidComponent T, typename... Args> decltype(auto) add_component(Args&&... args)
		{
			if (has_component<T>())
				return get_components<T>();
			return emplace_component<T>(std::forward<Args>(args)...);
		}

		template <ValidComponent T, typename... Args> decltype(auto) emplace_component(Args&&... args)
		{
			auto& registry = scene.get_registry();
			return registry.emplace<T>(identifier, std::forward<Args>(args)...);
		}

		template <DeletableComponent T> void remove_component()
		{
			if (!has_component<T>())
				return;
			auto& registry = scene.get_registry();
			registry.erase<T>(identifier);
		}

		void add_child(Entity&);
		void add_child(Entity* = nullptr);

		const auto& get_identifier() const { return identifier; }

	private:
		Scene& scene;
		std::string name;
		entt::entity identifier;
	};

} // namespace Disarray

template <> struct fmt::formatter<entt::entity> : fmt::formatter<std::string_view> {
	auto format(entt::entity entity, fmt::format_context& ctx) const -> decltype(ctx.out());
};

template <> struct fmt::formatter<Disarray::Entity> : fmt::formatter<std::string_view> {
	auto format(const Disarray::Entity& entity, fmt::format_context& ctx) const -> decltype(ctx.out());
};
