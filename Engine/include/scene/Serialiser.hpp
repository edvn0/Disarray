#pragma once

#include <fmt/core.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <tuple>
#include <utility>

#include "core/Collections.hpp"
#include "core/Hashes.hpp"
#include "core/Log.hpp"
#include "core/Tuple.hpp"
#include "scene/Component.hpp"
#include "scene/ComponentSerialisers.hpp"
#include "scene/Components.hpp"
#include "scene/Scene.hpp"
#include "util/Timer.hpp"

namespace Disarray {

namespace Detail {
	class CouldNotSerialiseException : public std::runtime_error {
	public:
		explicit CouldNotSerialiseException(const std::string& message)
			: runtime_error(message)
		{
		}
	};

	template <class T>
	concept SerialiserFor = requires(T t, ImmutableEntity& entity) {
		{
			t.can_serialise(entity)
		} -> std::same_as<bool>;
		{
			t.get_component_name()
		} -> std::convertible_to<std::string_view>;
	};

	template <SerialiserFor... Serialisers> struct Serialiser {
	private:
		std::tuple<Serialisers...> serialisers {};

	public:
		using json = nlohmann::json;

		explicit Serialiser(const Scene* input_scene, std::filesystem::path output_path = "Assets/Scene")
			: scene(input_scene)
			, path(std::move(output_path))
		{
			try {
				serialised_object = serialise();
			} catch (const CouldNotSerialiseException&) {
				return;
			}

			auto name = scene->get_name();
			std::replace(name.begin(), name.end(), ' ', '_');
			std::replace(name.begin(), name.end(), '+', '_');

			const auto epoch_count = std::chrono::system_clock::now().time_since_epoch().count();
			auto scene_name = fmt::format("{}-{}.json", name, epoch_count);
			auto full_path = path / scene_name;
			std::ofstream output { full_path };
			if (!output) {
				return;
			}

			output << std::setw(2) << serialised_object;
		};

		struct EntityAndKey {
			std::string key;
			json data;
		};

		auto serialise() -> json
		{
			json root;
			root["name"] = scene->get_name();

			const auto& registry = scene->get_registry();
			const auto view = registry.template view<const Components::ID, const Components::Tag>();

			static auto serialise_all = [&]<class... C>(Detail::ComponentGroup<C...>, const auto& entity, auto& components) {
				(serialise_component<C>(entity, components), ...);
			};

			std::vector<EntityAndKey> output;
			output.reserve(view.size_hint());
			view.each([this, &output](const auto handle, const auto& id, const auto& tag) {
				ImmutableEntity entity { scene, handle, tag.name };
				auto key = fmt::format("{}__disarray__{}", id.identifier, tag.name);
				json entity_object;
				json components;

				serialise_all(AllComponents {}, entity, components);

				entity_object["components"] = components;
				output.push_back({ key, entity_object });
			});

			json entities;
			Collections::for_each(output, [&e = entities](EntityAndKey k) { e[k.key] = k.data; });

			root["entities"] = entities;

			return root;
		}

		template <class T> void serialise_component(auto& entity, json& components)
		{
			static constexpr auto type = serialiser_type_for<T>;
			auto result = std::apply(
				[](auto... remaining) {
					return std::tuple_cat(
						std::conditional_t<(decltype(remaining)::type == type), std::tuple<decltype(remaining)>, std::tuple<>> {}...);
				},
				serialisers);
			Tuple::static_for(result, [&entity, &components](auto, auto& serialiser) {
				if (serialiser.can_serialise(entity)) {
					json object;
					auto& component = entity.template get_components<T>();
					auto key = serialiser.get_component_name();
					serialiser.serialise(component, object);
					components[key] = object;
				}
			});
		}

		auto get_as_json() const -> const auto& { return serialised_object; }

	private:
		const Scene* scene;
		std::filesystem::path path;
		json serialised_object;
	};
} // namespace Detail

using SceneSerialiser = Detail::Serialiser<ScriptSerialiser, MeshSerialiser, SkyboxSerialiser, TextSerialiser, BoxColliderSerialiser,
	SphereColliderSerialiser, CapsuleColliderSerialiser, ColliderMaterialSerialiser, RigidBodySerialiser, TextureSerialiser, TransformSerialiser,
	LineGeometrySerialiser, QuadGeometrySerialiser, DirectionalLightSerialiser, PointLightSerialiser, SpotLightSerialiser, InheritanceSerialiser>;

} // namespace Disarray
