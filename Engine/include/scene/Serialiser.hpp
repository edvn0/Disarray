#pragma once

#include "core/Log.hpp"
#include "scene/Component.hpp"
#include "scene/Components.hpp"
#include "scene/Entity.hpp"

#include <filesystem>
#include <tuple>
#include <yaml-cpp/yaml.h>

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
		void serialise(const T& component, YAML::Emitter& yaml_node) { static_cast<Child&>(*this).serialise_impl(component, yaml_node); };
	};

	struct PipelineSerialiser : public ComponentSerialiser<Components::Pipeline, PipelineSerialiser> {
		static constexpr SerialiserType serialiser_type = SerialiserType::Pipeline;
		void serialise_impl(const Components::Pipeline& pipeline, YAML::Emitter& out)
		{
			using namespace YAML;
			out << Key << "name";
			out << Value << "Pipeline";
			out << BeginMap;
			out << Key << "line_width";
			out << Value << pipeline.pipeline->get_properties().line_width;
			out << EndMap;
		}
	};

	namespace {
		template <class... Serialisers> struct Serialiser {
			Serialiser(const Scene& s, const std::filesystem::path& output = "Assets/Scene")
				: scene(s)
				, path(output)
			{
				serialise();
			};
			std::tuple<Serialisers...> serialisers {};

			void serialise()
			{
				using namespace YAML;
				Emitter out;
				out.SetIndent(2);
				out << BeginMap;
				out << Key << "name";
				out << Value << "Scene";
				out << Key << "components";
				out << Value << BeginSeq;

				const auto& registry = scene.get_registry();
				const auto view = registry.view<const Components::Pipeline>();

				auto pipeline_serialisers = filter<SerialiserType::Pipeline>();
				for (const auto [entity, pipeline] : view.each()) {
					static_for(pipeline_serialisers, [&out, &pipeline](std::size_t, auto& serialiser) { serialiser.serialise(pipeline, out); });
				}

				out << Value << EndSeq;
				out << EndMap;

				Log::debug("Scene Serialiser", "\n{}", out.c_str());
			}

			template <SerialiserType T> auto filter()
			{
				return std::apply(
					[](auto... ts) {
						return std::tuple_cat(std::conditional_t<(decltype(ts)::serialiser_type == T), std::tuple<decltype(ts)>, std::tuple<>> {}...);
					},
					serialisers);
			}

		private:
			const Scene& scene;
			std::filesystem::path path;
		};
	} // namespace

	using SceneSerialiser = Serialiser<PipelineSerialiser, PipelineSerialiser>;

} // namespace Disarray
