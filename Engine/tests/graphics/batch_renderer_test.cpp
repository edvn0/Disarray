#include "graphics/RenderBatch.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/VertexTypes.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/render_batch_implementation/RenderBatchImplementation.hpp"

#include <gtest/gtest.h>

using namespace Disarray;

struct ExecutorMock : public CommandExecutor {
	~ExecutorMock() override = default;

	void begin() override { }
	void end() override { }
	void submit_and_end() override { }
	float get_gpu_execution_time(uint32_t frame_index, uint32_t query_index) const override { return 0; }
	const PipelineStatistics& get_pipeline_statistics(uint32_t frame_index) const override { return stats; }
	bool has_stats() const override { return false; }

	PipelineStatistics stats {};
};

struct QueueFamilyIndexMock : public QueueFamilyIndex {
	~QueueFamilyIndexMock() override = default;
};

struct PhysicalDeviceMock : public PhysicalDevice {
	~PhysicalDeviceMock() override = default;

	QueueFamilyIndex& get_queue_family_indexes() override { return mock; }
	QueueFamilyIndexMock mock {};
};

struct DeviceMock : public Device {
	~DeviceMock() override = default;

	PhysicalDevice& get_physical_device() override { return mock; }
	PhysicalDeviceMock mock {};
};

struct RendererMock : public Renderer {
	~RendererMock() override = default;
	void begin_pass(CommandExecutor& executor, Framebuffer& framebuffer, bool explicit_clear) override { }
	void begin_pass(CommandExecutor& executor, Framebuffer& framebuffer) override { }
	void begin_pass(CommandExecutor& executor) override { }
	void end_pass(CommandExecutor& executor) override { }
	void on_resize() override { }
	PipelineCache& get_pipeline_cache() override { return c1; }
	TextureCache& get_texture_cache() override { return c2; }
	void begin_frame(Camera& camera) override { }
	void end_frame() override { }
	void force_recreation() override { }

	void expose_to_shaders(Image&) override {};
	VkDescriptorSet get_descriptor_set(std::uint32_t) override { return nullptr; }
	VkDescriptorSet get_descriptor_set() override { return nullptr; }
	const std::vector<VkDescriptorSetLayout>& get_descriptor_set_layouts() override { return layouts; }
	const PushConstant* get_push_constant() const override { return &pc; };
	PushConstant& get_editable_push_constant() override { return pc; };
	PushConstant pc;
	std::vector<VkDescriptorSetLayout> layouts;

	void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const GeometryProperties& = {}) override {};
	void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const glm::mat4& transform = glm::identity<glm::mat4>()) override {};
	void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const Disarray::Pipeline&,
		const glm::mat4& transform = glm::identity<glm::mat4>()) override {};

	void submit_batched_geometry(Disarray::CommandExecutor& executor) override
	{
		batch_renderer.submit(*this, executor);
		batch_renderer.reset();
	}

	void on_batch_full(std::function<void(Disarray::Renderer&)>&& func) override { on_batch_full_func = func; }

	void flush_batch(Disarray::CommandExecutor& executor) override
	{
		batch_renderer.submit(*this, executor);
		batch_renderer.reset();
	}

	void draw_planar_geometry(Geometry geometry, const GeometryProperties& properties) override
	{
		if (batch_renderer.is_full()) {
			on_batch_full_func(*this);
		}

		add_geometry_to_batch(geometry, properties);
	}

	void add_geometry_to_batch(Disarray::Geometry geometry, const Disarray::GeometryProperties& properties)
	{
		batch_renderer.create_new(geometry, properties);
		batch_renderer.submitted_geometries++;
	}

	ExecutorMock exec {};
	std::function<void(Disarray::Renderer&)> on_batch_full_func = [&exc = exec](Disarray::Renderer& renderer) { renderer.flush_batch(exc); };

	DeviceMock mock {};
	PipelineCache c1 { mock, "Test" };
	TextureCache c2 { mock, "Test" };
	static constexpr auto objects = 3;
	BatchRenderer<objects> batch_renderer {};
};

TEST(BatchRenderer, Construction)
{
	static constexpr auto objects = 2;
	BatchRenderer<objects> batch_renderer {};

	EXPECT_EQ(batch_renderer.submitted_geometries, 0) << "Submitted geometries isnt zero on construction";
	const auto& [quad_batch, line_batch] = batch_renderer.objects;
	EXPECT_EQ(quad_batch.vertices.size(), objects * 4);
	EXPECT_EQ(line_batch.vertices.size(), objects * 2);
}
