#pragma once

#include "core/Log.hpp"
#include "scene/Component.hpp"
#include "scene/Components.hpp"
#include "scene/Entity.hpp"

#include <filesystem>
#include <magic_enum.hpp>
#include <nlohmann/json.hpp>
#include <sstream>
#include <tuple>

namespace Disarray {

	class Scene;

	enum class SerialiserType {
		Faulty,
		Pipeline,
	};

	namespace {
		template <class Tup, class Func, std::size_t... Is> constexpr void static_for_impl(Tup&& t, Func&& f, std::index_sequence<Is...>)
		{
			(f(std::integral_constant<std::size_t, Is> {}, std::get<Is>(t)), ...);
		}

		template <class... T, class Func> constexpr void static_for(std::tuple<T...>& t, Func&& f)
		{
			static_for_impl(t, std::forward<Func>(f), std::make_index_sequence<sizeof...(T)> {});
		}
	} // namespace

	template <ValidComponent T, class Child> struct ComponentSerialiser {
		static constexpr SerialiserType serialiser_type = SerialiserType::Faulty;

		bool can_serialise(const Entity& entity) { return entity.has_component<T>(); }

		void serialise(const T& component, nlohmann::json& object_for_the_component)
		{
			static_cast<Child&>(*this).serialise_impl(component, object_for_the_component);
		};
	};

	struct PipelineSerialiser : public ComponentSerialiser<Components::Pipeline, PipelineSerialiser> {
		static constexpr SerialiserType serialiser_type = SerialiserType::Pipeline;
		void serialise_impl(const Components::Pipeline& pipeline, nlohmann::json&);
	};

	namespace {

		template <class... Serialisers> struct Serialiser {
			using json = nlohmann::json;

			explicit Serialiser(Scene& s, const std::filesystem::path& output = "Assets/Scene")
				: scene(s)
				, path(output)
			{
				serialise();
			};
			std::tuple<Serialisers...> serialisers {};

			void serialise()
			{
				json root;
				root["name"] = "Scene";

				const auto& registry = scene.get_registry();

				const auto view = registry.view<const ID, const Tag>();
				auto entities = json::array();
				for (const auto [handle, id, tag] : view.each()) {
					json entity_object;
					Entity entity { scene, handle };
					entity_object["tag"] = tag.name;
					entity_object["identifier"] = id.identifier;

					auto components = json::array();
					serialise_component<Components::Pipeline>(entity, components);

					entity_object["components"] = components;
					entities.push_back(entity_object);
				}

				root["entities"] = entities;
				std::stringstream stream;
				stream << std::setw(4) << root;
				Log::debug("Scene Serialiser", "\n{}", stream.str());
			}

			template <class T> void serialise_component(Entity& entity, json& components)
			{
				static_for(serialisers, [&entity, &components](auto, auto& serialiser) {
					if (serialiser.can_serialise(entity)) {
						json object;
						auto& component = entity.get_components<T>();
						serialiser.serialise(component, object);
						ensure(object.contains("component_name"), "Component name is missing.");
						components.push_back(object);
					}
				});
			}

		private:
			Scene& scene;
			std::filesystem::path path;
		};
	} // namespace

	using SceneSerialiser = Serialiser<PipelineSerialiser>;

} // namespace Disarray
