#pragma once

#include "core/Log.hpp"
#include "core/Tuple.hpp"
#include "scene/ComponentSerialisers.hpp"

#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <tuple>

namespace Disarray {

class Scene;

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
		using json = nlohmann::json;

		explicit Serialiser(Scene& s, const std::filesystem::path& output_path = "Assets/Scene")
			: scene(s)
			, path(output_path)
		{
			try {
				serialised_object = serialise();
			} catch (const CouldNotSerialiseException& exc) {
				Log::error("Scene Serialiser", "Could not serialise scene. Message: {}", exc.what());
				return;
			}

			if (serialised_object.empty()) {
				Log::info("Scene Serialiser", "Serialised output was empty...?");
				return;
			}

			namespace ch = std::chrono;
			auto time = Log::current_time(false);
			std::replace(time.begin(), time.end(), ':', '-');
			std::replace(time.begin(), time.end(), ' ', '-');
			auto name = scene.get_name();
			std::replace(name.begin(), name.end(), ' ', '_');
			std::replace(name.begin(), name.end(), '+', '_');

			auto scene_name = fmt::format("{}-{}.json", name, time);
			auto full_path = path / scene_name;
			std::ofstream output { full_path };
			if (!output) {
				Log::error("Scene Serialiser", "Could not open {} for writing.", full_path.string());
				return;
			}

			output << std::setw(2) << serialised_object;
			Log::info("Scene Serialiser", "Successfully serialised scene!");
		};

		std::tuple<Serialisers...> serialisers;

		json serialise()
		{
			json root;
			root["name"] = "Scene";

			const auto& registry = scene.get_registry();

			const auto view = registry.template view<const Components::ID, const Components::Tag>();
			json entities;
			for (const auto [handle, id, tag] : view.each()) {
				Entity entity { scene, handle, tag.name };
				auto key = fmt::format("{}__disarray__{}", id.identifier, tag.name);

				json entity_object;

				json components;
				serialise_component<Components::Pipeline>(entity, components);
				serialise_component<Components::Texture>(entity, components);
				serialise_component<Components::Mesh>(entity, components);
				serialise_component<Components::Transform>(entity, components);
				serialise_component<Components::LineGeometry>(entity, components);
				serialise_component<Components::QuadGeometry>(entity, components);
				serialise_component<Components::Inheritance>(entity, components);
				entity_object["components"] = components;

				entities[key] = entity_object;
			}

			root["entities"] = entities;

			return root;
		}

		template <class T> void serialise_component(Entity& entity, json& components)
		{
			static constexpr auto type = serialiser_type_for<T>;
			auto result = std::apply(
				[](auto... ts) {
					return std::tuple_cat(std::conditional_t<(decltype(ts)::type == type), std::tuple<decltype(ts)>, std::tuple<>> {}...);
				},
				serialisers);
			Tuple::static_for(result, [&entity, &components](auto, auto& serialiser) {
				if (serialiser.can_serialise(entity)) {
					json object;
					auto& component = entity.get_components<T>();
					auto key = serialiser.get_component_name();
					serialiser.serialise(component, object);
					components[key] = object;
				}
			});
		}

		const auto& get_as_json() const { return serialised_object; }

	private:
		Scene& scene;
		std::filesystem::path path;
		json serialised_object;
	};
} // namespace

using SceneSerialiser
	= Serialiser<TextureSerialiser, MeshSerialiser, TransformSerialiser, InheritanceSerialiser, LineGeometrySerialiser, QuadGeometrySerialiser>;

} // namespace Disarray
