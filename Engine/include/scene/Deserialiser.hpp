#pragma once

#include <nlohmann/json.hpp>

#include <exception>
#include <fstream>
#include <sstream>
#include <utility>

#include "core/Formatters.hpp"
#include "core/Tuple.hpp"
#include "core/exceptions/BaseException.hpp"
#include "scene/Component.hpp"
#include "scene/ComponentSerialisers.hpp"
#include "scene/Components.hpp"
#include "scene/Entity.hpp"

namespace Disarray {

namespace {

	class CouldNotDeserialiseException : public BaseException { };

	template <class... Deserialisers> struct Deserialiser {
		using json = nlohmann::json;

		explicit Deserialiser(Scene& input_scene, const Device& dev, std::istream& to_deserialise)
			: scene(input_scene)
			, device(dev)
		{
			json parsed = json::parse(to_deserialise);

			bool could_serialise { true };
			try {
				could_serialise = try_deserialise(parsed);
			} catch (const CouldNotDeserialiseException&) {
				return;
			}

			if (!could_serialise) {
				return;
			}
		}

		explicit Deserialiser(Scene& input_scene, const Device& dev, std::filesystem::path input_path)
			: scene(input_scene)
			, device(dev)
			, path(std::move(input_path))
		{
			json parsed;
			std::ifstream input_stream { path };
			if (!input_stream) {
				return;
			}

			parsed = json::parse(input_stream);

			bool could_serialise { true };
			could_serialise = try_deserialise(parsed);

			if (!could_serialise) {
				return;
			}
		};

		std::tuple<Deserialisers...> serialisers {};

		auto try_deserialise(const json& root) -> bool
		{
			auto&& entities = root["entities"];

			for (const auto& json_entity : entities.items()) {
				auto&& key = json_entity.key();

				auto&& components = json_entity.value()["components"];
				auto&& [id, tag] = parse_key(key);
				Entity entity;
				entity = Entity::deserialise(scene, id, tag);

				deserialise_component<Components::Script>(components, entity);
				deserialise_component<Components::Mesh>(components, entity);
				deserialise_component<Components::Skybox>(components, entity);
				deserialise_component<Components::Text>(components, entity);
				deserialise_component<Components::BoxCollider>(components, entity);
				deserialise_component<Components::SphereCollider>(components, entity);
				deserialise_component<Components::PillCollider>(components, entity);
				deserialise_component<Components::Texture>(components, entity);
				deserialise_component<Components::Transform>(components, entity);
				deserialise_component<Components::LineGeometry>(components, entity);
				deserialise_component<Components::QuadGeometry>(components, entity);
				deserialise_component<Components::DirectionalLight>(components, entity);
				deserialise_component<Components::PointLight>(components, entity);
				deserialise_component<Components::Inheritance>(components, entity);
			}

			return true;
		}

		auto parse_key(const json& json_key) -> std::pair<Identifier, std::string>
		{
			std::string key = json_key;
			static constexpr std::string_view split = "__disarray__";
			auto found = static_cast<long long>(key.find(split));

			std::string identifier { key.begin(), key.begin() + found };
			std::string tag { key.begin() + found + split.size(), key.end() };
			return { std::stoull(identifier), tag };
		}

		template <class T> void deserialise_component(const json& components, Entity& entity)
		{
			static constexpr auto type = serialiser_type_for<T>;
			auto result = std::apply(
				[](auto... types) {
					return std::tuple_cat(std::conditional_t<(decltype(types)::type == type), std::tuple<decltype(types)>, std::tuple<>> {}...);
				},
				serialisers);
			Tuple::static_for(result, [&entity, &components, &dev = device](auto, auto& deserialiser) {
				const auto key = deserialiser.get_component_name();
				if (components.contains(key) && deserialiser.should_add_component(components[key])) {
					auto& component = entity.add_component<T>();
					deserialiser.deserialise(components[key], component, dev);
				}
			});
		}

	private:
		Scene& scene;
		const Disarray::Device& device;
		std::filesystem::path path;
	};
} // namespace

using SceneDeserialiser = Deserialiser<ScriptDeserialiser, MeshDeserialiser, SkyboxDeserialiser, TextDeserialiser, BoxColliderDeserialiser,
	SphereColliderDeserialiser, PillColliderDeserialiser, TextureDeserialiser, TransformDeserialiser, LineGeometryDeserialiser,
	QuadGeometryDeserialiser, DirectionalLightDeserialiser, PointLightDeserialiser, InheritanceDeserialiser>;

} // namespace Disarray
