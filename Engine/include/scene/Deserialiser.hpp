#pragma once

#include <nlohmann/json.hpp>

#include <exception>
#include <fstream>
#include <sstream>
#include <type_traits>
#include <utility>

#include "core/Collections.hpp"
#include "core/Formatters.hpp"
#include "core/Tuple.hpp"
#include "core/exceptions/BaseException.hpp"
#include "scene/Component.hpp"
#include "scene/ComponentSerialisers.hpp"
#include "scene/Components.hpp"
#include "scene/Entity.hpp"

namespace Disarray {

namespace Detail {

	class CouldNotDeserialiseException : public BaseException {
		using BaseException::BaseException;
	};

	template <class C> class DeserialiserCache {
	public:
		DeserialiserCache() = default;

		auto get_cached(const std::string& key) -> Ref<C>
		{
			if (cache.contains(key)) {
				return cache.at(key);
			}

			return nullptr;
		}

		auto cache_object(const std::string& key, const Ref<C>& mesh) -> void { cache[key] = mesh; }

		auto clear_cache() -> void { cache.clear(); }

	private:
		Collections::ReferencedStringMap<C> cache {};
	};

	using MeshCache = DeserialiserCache<Components::Mesh>;
	using TextureCache = DeserialiserCache<Components::Texture>;

	template <ValidComponent C> struct DeserialiseComponent {
		auto operator()(auto& serialisers, const auto& device, const nlohmann::json& components, Entity& entity)
		{
			static constexpr auto type = serialiser_type_for<C>;
			auto result = std::apply(
				[](auto... types) {
					return std::tuple_cat(std::conditional_t<(decltype(types)::type == type), std::tuple<decltype(types)>, std::tuple<>> {}...);
				},
				serialisers);
			Tuple::static_for(result, [&entity, &components, &dev = device](auto, auto& deserialiser) {
				const auto key = deserialiser.get_component_name();
				if (components.contains(key) && deserialiser.should_add_component(components[key])) {
					auto& component = entity.add_component<C>();
					deserialiser.deserialise(components[key], component, dev);
				}
			});
		}
	};

	template <> struct DeserialiseComponent<Components::Mesh> {
		auto operator()(auto& serialisers, const auto& device, const nlohmann::json& components, Entity& entity)
		{
			static constexpr auto type = serialiser_type_for<Components::Mesh>;
			auto result = std::apply(
				[](auto... types) {
					return std::tuple_cat(std::conditional_t<(decltype(types)::type == type), std::tuple<decltype(types)>, std::tuple<>> {}...);
				},
				serialisers);
			Tuple::static_for(result, [&entity, &components, &dev = device, &cache = mesh_cache](auto, auto& deserialiser) {
				const auto key = deserialiser.get_component_name();
				if (components.contains(key) && deserialiser.should_add_component(components[key])) {
					auto& component = entity.add_component<Components::Mesh>();
					const std::filesystem::path& path = components[key]["properties"]["path"];
					const auto& as_string = path.filename().string();
					auto maybe_mesh = cache.get_cached(as_string);
					const auto contained = maybe_mesh != nullptr;

					if (!contained) {
						deserialiser.deserialise(components[key], component, dev);
						cache.cache_object(as_string, component.mesh);
					} else {
						component.mesh = maybe_mesh;
					}
				}
			});
		}

	private:
		DeserialiserCache<Disarray::Mesh> mesh_cache;
	};

	using SpecialisedDeserialisers = std::tuple<DeserialiseComponent<Components::Transform>, DeserialiseComponent<Components::Inheritance>,
		DeserialiseComponent<Components::LineGeometry>, DeserialiseComponent<Components::QuadGeometry>, DeserialiseComponent<Components::Mesh>,
		DeserialiseComponent<Components::Material>, DeserialiseComponent<Components::Texture>, DeserialiseComponent<Components::DirectionalLight>,
		DeserialiseComponent<Components::PointLight>, DeserialiseComponent<Components::SpotLight>, DeserialiseComponent<Components::Script>,
		DeserialiseComponent<Components::Controller>, DeserialiseComponent<Components::Camera>, DeserialiseComponent<Components::BoxCollider>,
		DeserialiseComponent<Components::SphereCollider>, DeserialiseComponent<Components::CapsuleCollider>,
		DeserialiseComponent<Components::ColliderMaterial>, DeserialiseComponent<Components::RigidBody>, DeserialiseComponent<Components::Skybox>,
		DeserialiseComponent<Components::Text>>;

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

			SpecialisedDeserialisers deserialisers {};
			for (const auto& json_entity : entities.items()) {
				auto&& key = json_entity.key();

				auto&& components = json_entity.value()["components"];
				auto&& [id, tag] = parse_key(key);
				Entity entity = Entity::deserialise(scene, id, tag);

				Tuple::static_for(deserialisers, [&](auto, auto& specialised) { specialised(serialisers, device, components, entity); });
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

	private:
		Scene& scene;
		const Disarray::Device& device;
		std::filesystem::path path;
	};
} // namespace Detail

using SceneDeserialiser = Detail::Deserialiser<ScriptDeserialiser, MeshDeserialiser, SkyboxDeserialiser, TextDeserialiser, BoxColliderDeserialiser,
	SphereColliderDeserialiser, CapsuleColliderDeserialiser, ColliderMaterialDeserialiser, RigidBodyDeserialiser, TextureDeserialiser,
	TransformDeserialiser, LineGeometryDeserialiser, QuadGeometryDeserialiser, DirectionalLightDeserialiser, PointLightDeserialiser,
	InheritanceDeserialiser>;

} // namespace Disarray
