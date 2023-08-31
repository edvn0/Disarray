#pragma once

#include <array>

#include "core/Tuple.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Device.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/VertexBuffer.hpp"
#include "graphics/VertexTypes.hpp"
#include "scene/Camera.hpp"

namespace Disarray {

#ifdef DISARRAY_BATCH_RENDERER_SIZE
static constexpr std::size_t batch_renderer_size = DISARRAY_BATCH_RENDERER_SIZE;
#else
static constexpr std::size_t batch_renderer_size = 500;
#endif

template <IsValidVertexType T, typename Child = std::enable_if<vertex_per_object_count<T> != 0 && index_per_object_count<T> != 0>>
struct RenderBatchFor {
	static constexpr auto VertexCount = vertex_per_object_count<T>;
	static constexpr auto IndexCount = index_per_object_count<T>;

	std::array<T, batch_renderer_size * VertexCount> vertices {};
	Scope<Disarray::IndexBuffer> index_buffer { nullptr };
	Scope<Disarray::VertexBuffer> vertex_buffer { nullptr };

	// From the pipeline cache!
	Ref<Disarray::Pipeline> pipeline { nullptr };

	std::uint32_t submitted_vertices { 0 };
	std::uint32_t submitted_indices { 0 };
	std::uint32_t submitted_objects { 0 };

	void construct(Disarray::Renderer& renderer, const Disarray::Device& device) { static_cast<Child&>(*this).construct_impl(renderer, device); }
	void submit(Disarray::Renderer& renderer, Disarray::CommandExecutor& executor) { static_cast<Child&>(*this).submit_impl(renderer, executor); }
	void create_new(Geometry geometry, const GeometryProperties& properties) { static_cast<Child&>(*this).create_new_impl(geometry, properties); }
	void flush(Disarray::Renderer& renderer, Disarray::CommandExecutor& executor) { static_cast<Child&>(*this).flush_impl(renderer, executor); }

	void reset()
	{
		vertices.fill({});
		submitted_indices = 0;
		submitted_vertices = 0;
		submitted_objects = 0;
	}

protected:
	void prepare_data() { vertex_buffer->set_data(vertices.data(), submitted_vertices * sizeof(T)); }

	void flush_vertex_buffer() { vertex_buffer->set_data(vertices.data(), static_cast<std::uint32_t>(vertex_buffer_size())); }

	auto vertex_buffer_size() -> std::size_t { return VertexCount * batch_renderer_size * sizeof(T); }
	auto index_buffer_size() -> std::size_t { return IndexCount * batch_renderer_size * sizeof(std::uint32_t); }

	auto emplace() -> T& { return vertices[submitted_vertices++]; }
};

#define MAKE_BATCH_RENDERER(Type, Child)                                                                                                             \
	using Current = RenderBatchFor<Type, Child>;                                                                                                     \
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

struct LineVertexBatch final : public RenderBatchFor<LineVertex, LineVertexBatch> {
	MAKE_BATCH_RENDERER(LineVertex, LineVertexBatch)

	void construct_impl(Disarray::Renderer&, const Disarray::Device&);
	void submit_impl(Disarray::Renderer&, Disarray::CommandExecutor&);
	void create_new_impl(Geometry, const GeometryProperties&);
	void flush_impl(Disarray::Renderer&, Disarray::CommandExecutor&);
};

struct QuadVertexBatch final : public RenderBatchFor<QuadVertex, QuadVertexBatch> {
	MAKE_BATCH_RENDERER(QuadVertex, QuadVertexBatch)

	void construct_impl(Disarray::Renderer&, const Disarray::Device&);
	void submit_impl(Disarray::Renderer&, Disarray::CommandExecutor&);
	void create_new_impl(Geometry, const GeometryProperties&);
	void flush_impl(Disarray::Renderer&, Disarray::CommandExecutor&);
};

struct LineIdVertexBatch final : public RenderBatchFor<LineIdVertex, LineIdVertexBatch> {
	MAKE_BATCH_RENDERER(LineIdVertex, LineIdVertexBatch)

	void construct_impl(Disarray::Renderer&, const Disarray::Device&);
	void submit_impl(Disarray::Renderer&, Disarray::CommandExecutor&);
	void create_new_impl(Geometry, const GeometryProperties&);
	void flush_impl(Disarray::Renderer&, Disarray::CommandExecutor&);
};

struct BatchRenderer {
	static constexpr auto Objects = batch_renderer_size;

	using Quads = QuadVertexBatch;
	using Lines = LineVertexBatch;
	using LinesWithIdentifiers = LineIdVertexBatch;

	std::tuple<Quads, Lines, LinesWithIdentifiers> objects {};

	// How many times have we submitted geometries?
	// Used by shaders to determine scale of picking count
	std::uint32_t submitted_geometries { 0 };

	constexpr void reset()
	{
		submitted_geometries = 0;
		Tuple::static_for(objects, [](std::size_t index, auto& batch) { batch.reset(); });
	}

	constexpr void construct(Disarray::Renderer& renderer, const Disarray::Device& device)
	{
		Tuple::static_for(objects, [&renderer, &device](std::size_t index, auto& batch) { batch.construct(renderer, device); });
	}

	auto would_be_full() -> bool
	{
		bool batch_would_be_full = false;
		Tuple::static_for(objects, [&batch_would_be_full](std::size_t index, auto& batch) {
			const auto more_than_max = batch.submitted_objects + 1 >= Objects;
			batch_would_be_full |= more_than_max;
		});
		return batch_would_be_full;
	}

	auto is_full() -> bool
	{
		bool batch_would_be_full = false;
		Tuple::static_for(objects, [&batch_would_be_full](std::size_t index, auto& batch) {
			const auto more_than_max = batch.submitted_objects >= Objects;
			batch_would_be_full |= more_than_max;
		});
		return batch_would_be_full;
	}

	auto should_submit() -> bool
	{
		bool should_submit_batch = false;
		Tuple::static_for(objects, [&should_submit_batch](std::size_t index, auto& batch) {
			const auto predicate = batch.submitted_objects > 0 && batch.submitted_objects < Objects;
			should_submit_batch |= predicate;
		});
		return should_submit_batch;
	}

	void flush(Renderer& renderer, CommandExecutor& executor)
	{
		submitted_geometries = 0;
		Tuple::static_for(objects, [&ren = renderer, &ce = executor](std::size_t index, auto& batch) mutable { batch.flush(ren, ce); });
	}

	constexpr void create_new(Geometry geometry, const GeometryProperties& properties)
	{
		Tuple::static_for(objects, [geometry, &properties](std::size_t index, auto& batch) mutable { batch.create_new(geometry, properties); });
	}

	constexpr void submit(Disarray::Renderer& renderer, Disarray::CommandExecutor& command_executor)
	{
		Tuple::static_for(
			objects, [&renderer, &command_executor](std::size_t index, auto& batch) mutable { batch.submit(renderer, command_executor); });
	}
};

} // namespace Disarray
