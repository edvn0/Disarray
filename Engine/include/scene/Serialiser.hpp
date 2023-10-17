#pragma once

#include <fmt/core.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <tuple>

#include "core/Collections.hpp"
#include "core/Hashes.hpp"
#include "core/Log.hpp"
#include "core/Tuple.hpp"
#include "scene/ComponentSerialisers.hpp"
#include "scene/Components.hpp"
#include "scene/Scene.hpp"
#include "util/Timer.hpp"

namespace Disarray {

namespace {
	class CouldNotSerialiseException : public std::runtime_error {
	public:
		explicit CouldNotSerialiseException(const std::string& message)
			: runtime_error(message)
		{
		}
		~CouldNotSerialiseException() noexcept override = default;
	};

	template <class... Serialisers> struct Serialiser {
	private:
		std::tuple<Serialisers...> serialisers {};

	public:
		using json = nlohmann::json;

		explicit Serialiser(const Scene* input_scene, const std::filesystem::path& output_path = "Assets/Scene")
			: scene(input_scene)
			, path(output_path)
		{
			try {
				serialised_object = serialise();
			} catch (const CouldNotSerialiseException&) {
				return;
			}

			namespace ch = std::chrono;
			auto name = scene->get_name();
			std::replace(name.begin(), name.end(), ' ', '_');
			std::replace(name.begin(), name.end(), '+', '_');

			auto scene_name = fmt::format("{}-{}.json", name, std::chrono::system_clock::now().time_since_epoch().count());
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
			root["name"] = "Scene";

			const auto& registry = scene->get_registry();
			const auto view = registry.template view<const Components::ID, const Components::Tag>();

			MSTimer timer {};
			std::vector<EntityAndKey> output;
			output.reserve(view.size_hint());
			view.each([this, &output](const auto handle, const auto& id, const auto& tag) {
				ImmutableEntity entity { scene, handle, tag.name };
				auto key = fmt::format("{}__disarray__{}", id.identifier, tag.name);
				json entity_object;
				json components;
				serialise_component<Components::Pipeline>(entity, components);
				serialise_component<Components::Texture>(entity, components);
				serialise_component<Components::Script>(entity, components);
				serialise_component<Components::Mesh>(entity, components);
				serialise_component<Components::Transform>(entity, components);
				serialise_component<Components::LineGeometry>(entity, components);
				serialise_component<Components::QuadGeometry>(entity, components);
				serialise_component<Components::Inheritance>(entity, components);
				serialise_component<Components::DirectionalLight>(entity, components);
				serialise_component<Components::PointLight>(entity, components);
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
} // namespace

using SceneSerialiser = Serialiser<PipelineSerialiser, ScriptSerialiser, TextureSerialiser, MeshSerialiser, TransformSerialiser,
	InheritanceSerialiser, LineGeometrySerialiser, QuadGeometrySerialiser, DirectionalLightSerialiser, PointLightSerialiser>;

} // namespace Disarray
