#include "graphics/Renderer.hpp"
#include "graphics/VertexTypes.hpp"
#include "vulkan/RenderBatch.hpp"
#include "vulkan/SpecialisedRenderBatch.hpp"

#include <gtest/gtest.h>

using namespace Disarray;

TEST(BatchRenderer, Construction)
{
	static constexpr auto objects = 2;
	BatchRenderer<objects, QuadVertex, LineVertex> batch_renderer {};

	EXPECT_EQ(batch_renderer.submitted_geometries, 0) << "Submitted geometries isnt zero on construction";
	const auto& [quad_batch, line_batch] = batch_renderer.objects;
	EXPECT_EQ(quad_batch.vertices.size(), objects * 4);
	EXPECT_EQ(line_batch.vertices.size(), objects * 2);
}

TEST(BatchRenderer, Iteration)
{
	static constexpr auto objects = 3;
	BatchRenderer<objects, QuadVertex, LineVertex> batch_renderer {};

	EXPECT_EQ(batch_renderer.submitted_geometries, 0);
	batch_renderer.create_new(Geometry::Rectangle, {});
	EXPECT_EQ(batch_renderer.submitted_geometries, 1);

	EXPECT_EQ(batch_renderer.submitted_geometries, 1);
	batch_renderer.create_new(Geometry::Rectangle, {});
	EXPECT_EQ(batch_renderer.submitted_geometries, 2);
}
