#pragma once

#include "graphics/CommandExecutor.hpp"
#include "graphics/Device.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/VertexBuffer.hpp"
#include "graphics/VertexTypes.hpp"

#include <array>

namespace Disarray {

	namespace Detail {
		template <class Tup, class Func, std::size_t... Is> constexpr void static_for_impl(Tup&& t, Func&& f, std::index_sequence<Is...>)
		{
			(f(std::integral_constant<std::size_t, Is> {}, std::get<Is>(t)), ...);
		}

		template <class... T, class Func> constexpr void static_for(std::tuple<T...>& t, Func&& f)
		{
			static_for_impl(t, std::forward<Func>(f), std::make_index_sequence<sizeof...(T)> {});
		}
	} // namespace Detail

	static constexpr auto max_objects = 200;

	template <IsValidVertexType T, std::size_t Objects = max_objects, std::size_t VertexCount = vertex_per_object_count<T>,
		std::size_t IndexCount = index_per_object_count<T>>
		requires(std::is_default_constructible_v<T> && VertexCount != 0 && IndexCount != 0)
	struct RenderBatchFor {
		// We want for example to be able to write 100 quads, which requires 100 * vertex_count(Quad) = 4 vertices
		std::array<T, Objects * VertexCount> vertices {};
		Scope<Disarray::IndexBuffer> index_buffer { nullptr };
		Scope<Disarray::VertexBuffer> vertex_buffer { nullptr };

		// From the pipeline cache!
		Ref<Vulkan::Pipeline> pipeline { nullptr };

		std::uint32_t submitted_vertices { 0 };
		std::uint32_t submitted_indices { 0 };
		std::uint32_t submitted_objects { 0 };

		void construct(Disarray::Renderer&, Disarray::Device&);
		void submit(Disarray::Renderer&, Disarray::CommandExecutor&);
		void create_new(Geometry, const GeometryProperties&);

		void reset()
		{
			T default_construction;
			vertices.fill(default_construction);
			submitted_indices = 0;
			submitted_vertices = 0;
			submitted_objects = 0;
		}

	private:
		void prepare_data() { vertex_buffer->set_data(vertices.data(), submitted_vertices * sizeof(T)); }

		std::size_t buffer_size() { return VertexCount * max_objects * sizeof(T); }

		T& emplace() { return vertices[submitted_vertices++]; }
	};

	template <std::size_t Objects = max_objects, IsValidVertexType... T> struct BatchRenderer {
		std::tuple<RenderBatchFor<T, Objects, vertex_per_object_count<T>, index_per_object_count<T>>...> objects {};

		// How many times have we submitted geometries?
		// Used by shaders to determine scale of picking count
		std::uint32_t submitted_geometries { 0 };

		constexpr void reset()
		{
			submitted_geometries = 0;
			Detail::static_for(objects, [](std::size_t index, auto& batch) { batch.reset(); });
		}

		constexpr void flush()
		{
			submitted_geometries = 0;
			Detail::static_for(objects, [](std::size_t index, auto& batch) { batch.flush(); });
		}

		constexpr void construct(Disarray::Renderer& renderer, Disarray::Device& device)
		{
			Detail::static_for(objects, [&renderer, &device](std::size_t index, auto& batch) { batch.construct(renderer, device); });
		}

		constexpr bool is_full()
		{
			bool batch_would_be_full = false;
			Detail::static_for(
				objects, [&batch_would_be_full](std::size_t index, auto& batch) { batch_would_be_full |= batch.submitted_objects >= Objects; });
			return batch_would_be_full;
		}

		constexpr void create_new(Geometry geometry, const GeometryProperties& properties)
		{
			Detail::static_for(objects, [geometry, &properties](std::size_t index, auto& batch) { batch.create_new(geometry, properties); });
		}

		constexpr void submit(Disarray::Renderer& renderer, Disarray::CommandExecutor& command_executor)
		{
			Detail::static_for(objects, [&renderer, &command_executor](std::size_t index, auto& batch) { batch.submit(renderer, command_executor); });
		}
	};

} // namespace Disarray
