#pragma once

#include "core/Formatters.hpp"
#include "core/Tuple.hpp"
#include "scene/Component.hpp"
#include "scene/ComponentSerialisers.hpp"
#include "scene/Entity.hpp"

#include <nlohmann/json.hpp>

namespace Disarray {

	namespace {

		class CouldNotDeserialiseException : public std::runtime_error {
		public:
			explicit CouldNotDeserialiseException(const std::string& message)
				: runtime_error(message)
			{
			}
			~CouldNotDeserialiseException() noexcept override = default;
		};

		template <class... Deserialisers> struct Deserialiser {
			using json = nlohmann::json;

			explicit Deserialiser(Scene& s, const Device& dev, const std::filesystem::path& input_path)
				: scene(s)
				, device(dev)
				, path(input_path)
			{
				json in;
				std::ifstream input_stream { path };
				if (!input_stream) {
					Log::error("Scene Deserialiser", "Could not open file at {}", path);
					return;
				}

				in = json::parse(input_stream);

				bool could_serialise;
				try {
					could_serialise = try_deserialise(in);
				} catch (const CouldNotDeserialiseException& exc) {
					Log::error("Scene Deserialiser", "Could not serialise scene. Message: {}", exc.what());
					return;
				}

				if (!could_serialise) {
					Log::info("Scene Deserialiser", "Deserialised output was empty...?");
					return;
				}
			};

			std::tuple<Deserialisers...> serialisers {};

			bool try_deserialise(const json& root)
			{
				auto& registry = scene.get_registry();

				auto&& scene_name = root["name"];
				auto&& entities = root["entities"];

				for (const auto& json_entity : entities.items()) {
					auto&& key = json_entity.key();

					auto&& components = json_entity.value()["components"];
					auto&& [id, tag] = parse_key(key);
					auto entity = Entity::deserialise(scene, registry.create(), id, tag);
					deserialise_component<Components::QuadGeometry>(components, entity, device);
					deserialise_component<Components::LineGeometry>(components, entity, device);
					deserialise_component<Components::Transform>(components, entity, device);
					deserialise_component<Components::Pipeline>(components, entity, device);
					deserialise_component<Components::Texture>(components, entity, device);
					deserialise_component<Components::Mesh>(components, entity, device);
					deserialise_component<Components::Inheritance>(components, entity, device);
				}

				return true;
			}

			std::pair<Identifier, std::string> parse_key(const json& k)
			{
				std::string key = k;
				static constexpr std::string_view split = "__disarray__";
				auto found = key.find(split);

				std::string id { key.begin(), key.begin() + found };
				std::string tag { key.begin() + found + split.size(), key.end() };
				return { std::stoull(id), tag };
			}

			template <class T> void deserialise_component(const json& components, Entity& entity, const Device& device)
			{
				static constexpr auto type = serialiser_type_for<T>;
				auto result = std::apply(
					[](auto... ts) {
						return std::tuple_cat(std::conditional_t<(decltype(ts)::type == type), std::tuple<decltype(ts)>, std::tuple<>> {}...);
					},
					serialisers);
				Tuple::static_for(result, [&entity, &components, &device](auto, auto& deserialiser) {
					const auto key = deserialiser.get_component_name();
					if (components.contains(key) && deserialiser.should_add_component(components[key])) {
						auto& component = entity.add_component<T>();
						deserialiser.deserialise(components[key], component, device);
					}
				});
			}

		private:
			Scene& scene;
			const Device& device;
			std::filesystem::path path;
		};
	} // namespace

	using SceneDeserialiser = Deserialiser<TextureDeserialiser, MeshDeserialiser, TransformDeserialiser, InheritanceDeserialiser,
		LineGeometryDeserialiser, QuadGeometryDeserialiser>;

} // namespace Disarray
