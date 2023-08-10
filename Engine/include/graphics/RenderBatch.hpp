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

	template <IsValidVertexType T, std::size_t Objects = 1,
		typename Child = std::enable_if<vertex_per_object_count<T> != 0 && index_per_object_count<T> != 0>>
	struct RenderBatchFor {
		static constexpr auto VertexCount = vertex_per_object_count<T>;
		static constexpr auto IndexCount = index_per_object_count<T>;

		std::array<T, Objects * VertexCount> vertices {};
		Scope<Disarray::IndexBuffer> index_buffer { nullptr };
		Scope<Disarray::VertexBuffer> vertex_buffer { nullptr };

		// From the pipeline cache!
		Ref<Disarray::Pipeline> pipeline { nullptr };

		std::uint32_t submitted_vertices { 0 };
		std::uint32_t submitted_indices { 0 };
		std::uint32_t submitted_objects { 0 };

		void construct(Disarray::Renderer& renderer, Disarray::Device& device) { static_cast<Child&>(*this).construct_impl(renderer, device); }
		void submit(Disarray::Renderer& renderer, Disarray::CommandExecutor& executor) { static_cast<Child&>(*this).submit_impl(renderer, executor); }
		void create_new(Geometry geometry, const GeometryProperties& properties) { static_cast<Child&>(*this).create_new_impl(geometry, properties); }
		void flush(Disarray::Renderer& renderer, Disarray::CommandExecutor& executor) { static_cast<Child&>(*this).flush_impl(renderer, executor); }

		void reset()
		{
			std::memset(vertices.data(), 0, vertices.size() * sizeof(T));
			submitted_indices = 0;
			submitted_vertices = 0;
			submitted_objects = 0;
		}

	protected:
		void prepare_data() { vertex_buffer->set_data(vertices.data(), submitted_vertices * sizeof(T)); }

		void flush_vertex_buffer() { vertex_buffer->set_data(vertices.data(), static_cast<std::uint32_t>(vertex_buffer_size())); }

		std::size_t vertex_buffer_size() { return VertexCount * Objects * sizeof(T); }
		std::size_t index_buffer_size() { return IndexCount * Objects * sizeof(std::uint32_t); }

		T& emplace() { return vertices[submitted_vertices++]; }
	};

#define MAKE_BATCH_RENDERER(Type, Child)                                                                                                             \
	using Current = RenderBatchFor<Type, Objects, Child<Objects>>;                                                                                   \
	using Current::emplace;                                                                                                                          \
	using Current::index_buffer;                                                                                                                     \
	using Current::index_buffer_size;                                                                                                                \
	using Current::IndexCount;                                                                                                                       \
	using Current::pipeline;                                                                                                                         \
	using Current::prepare_data;                                                                                                                     \
	using Current::reset;                                                                                                                            \
	using Current::submitted_indices;                                                                                                                \
	using Current::submitted_objects;                                                                                                                \
	using Current::submitted_vertices;                                                                                                               \
	using Current::vertex_buffer;                                                                                                                    \
	using Current::vertex_buffer_size;                                                                                                               \
	using Current::VertexCount;                                                                                                                      \
	using Current::vertices;                                                                                                                         \
	using Current::flush_vertex_buffer;

	template <std::size_t Objects = 1> struct LineVertexBatch final : public RenderBatchFor<LineVertex, Objects, LineVertexBatch<Objects>> {
		MAKE_BATCH_RENDERER(LineVertex, LineVertexBatch)

		void construct_impl(Disarray::Renderer&, Disarray::Device&);
		void submit_impl(Disarray::Renderer&, Disarray::CommandExecutor&);
		void create_new_impl(Geometry, const GeometryProperties&);
		void flush_impl(Disarray::Renderer&, Disarray::CommandExecutor&);
	};

	template <std::size_t Objects = 1> struct QuadVertexBatch final : public RenderBatchFor<QuadVertex, Objects, QuadVertexBatch<Objects>> {
		MAKE_BATCH_RENDERER(QuadVertex, QuadVertexBatch)

		void construct_impl(Disarray::Renderer&, Disarray::Device&);
		void submit_impl(Disarray::Renderer&, Disarray::CommandExecutor&);
		void create_new_impl(Geometry, const GeometryProperties&);
		void flush_impl(Disarray::Renderer&, Disarray::CommandExecutor&);
	};

	template <std::size_t Objects = 1> struct BatchRenderer {
		using Quads = QuadVertexBatch<Objects>;
		using Lines = LineVertexBatch<Objects>;

		std::tuple<Quads, Lines> objects {};

		// How many times have we submitted geometries?
		// Used by shaders to determine scale of picking count
		std::uint32_t submitted_geometries { 0 };

		constexpr void reset()
		{
			submitted_geometries = 0;
			Detail::static_for(objects, [](std::size_t index, auto& batch) { batch.reset(); });
		}

		constexpr void construct(Disarray::Renderer& renderer, Disarray::Device& device)
		{
			Detail::static_for(objects, [&renderer, &device](std::size_t index, auto& batch) { batch.construct(renderer, device); });
		}

		bool would_be_full()
		{
			bool batch_would_be_full = false;
			Detail::static_for(objects, [&batch_would_be_full](std::size_t index, auto& batch) {
				const auto more_than_max = batch.submitted_objects + 1 >= Objects;
				batch_would_be_full |= more_than_max;
			});
			return batch_would_be_full;
		}

		bool is_full()
		{
			bool batch_would_be_full = false;
			Detail::static_for(objects, [&batch_would_be_full](std::size_t index, auto& batch) {
				const auto more_than_max = batch.submitted_objects >= Objects;
				batch_would_be_full |= more_than_max;
			});
			return batch_would_be_full;
		}

		bool should_submit()
		{
			bool should_submit_batch = false;
			Detail::static_for(objects, [&should_submit_batch](std::size_t index, auto& batch) {
				const auto predicate = batch.submitted_objects > 0 && batch.submitted_objects < Objects;
				should_submit_batch |= predicate;
			});
			return should_submit_batch;
		}

		void flush(Renderer& renderer, CommandExecutor& executor)
		{
			submitted_geometries = 0;
			Detail::static_for(objects, [&ren = renderer, &ce = executor](std::size_t index, auto& batch) mutable { batch.flush(ren, ce); });
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
