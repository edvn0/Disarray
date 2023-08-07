#pragma once

#include "scene/Component.hpp"
#include "scene/Scene.hpp"

#include <entt/entt.hpp>
#include <string>
#include <string_view>

namespace Disarray {

	class Entity {
	public:
		Entity(Scene&, std::string_view = "Empty");
		Entity(Scene&, entt::entity, std::string_view = "Empty");

		template <ValidComponent... T> decltype(auto) has_any() { return scene.get_registry().any_of<T...>(identifier); }
		template <ValidComponent... T> decltype(auto) has_all() { return scene.get_registry().all_of<T...>(identifier); }
		template <ValidComponent... T> decltype(auto) get_components() { return scene.get_registry().get<T...>(identifier); }
		template <ValidComponent T> decltype(auto) has_component() { return scene.get_registry().any_of<T>(identifier); }
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

		void add_child(Entity);

	private:
		Scene& scene;
		std::string name;
		entt::entity identifier;
	};

} // namespace Disarray
