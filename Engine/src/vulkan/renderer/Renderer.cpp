#include "DisarrayPCH.hpp"

#include "vulkan/Renderer.hpp"

#include "core/Types.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Swapchain.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Framebuffer.hpp"
#include "vulkan/IndexBuffer.hpp"
#include "vulkan/Mesh.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/UniformBuffer.hpp"
#include "vulkan/VertexBuffer.hpp"
#include "vulkan/render_batch_implementation/RenderBatchImplementation.hpp"

#include <array>
#include <core/Clock.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Disarray::Vulkan {

	Renderer::Renderer(Disarray::Device& dev, Disarray::Swapchain& sc, const Disarray::RendererProperties& properties)
		: device(dev)
		, swapchain(sc)
		, pipeline_cache(dev, "Assets/Shaders")
		, texture_cache(dev, "Assets/Textures")
		, props(properties)
		, extent(swapchain.get_extent())
	{
		frame_ubos.resize(swapchain.image_count());
		for (auto& ubo : frame_ubos) {
			ubo = UniformBuffer::construct(device,
				BufferProperties {
					.size = sizeof(UBO),
					.binding = 0,
				});
		}

		initialise_descriptors();

		FramebufferProperties geometry_props { .extent = swapchain.get_extent(),
			.attachments = { { ImageFormat::SBGR }, { ImageFormat::Depth } },
			.clear_colour_on_load = false,
			.clear_depth_on_load = false,
			.debug_name = "RendererFramebuffer" };
		geometry_framebuffer = Framebuffer::construct(device, geometry_props);

		quad_framebuffer = Framebuffer::construct(device,
			{ .extent = swapchain.get_extent(),
				.attachments = { { ImageFormat::SBGR },  { ImageFormat::Uint, false },{ ImageFormat::Depth }, },
				.debug_name = "QuadFramebuffer" });

		PipelineCacheCreationProperties pipeline_properties = {
			.pipeline_key = "quad",
			.vertex_shader_key = "quad.vert",
			.fragment_shader_key = "quad.frag",
			.framebuffer = geometry_framebuffer,
			.layout = { LayoutElement { ElementType::Float3, "position" }, { ElementType::Float2, "uvs" }, { ElementType::Float2, "normals" },
				{ ElementType::Float4, "colour" }, { ElementType::Uint, "identifier" } },
			.push_constant_layout = PushConstantLayout { PushConstantRange { PushConstantKind::Both, sizeof(PushConstant) } },
			.extent = swapchain.get_extent(),
			.descriptor_set_layout = layouts.data(),
			.descriptor_set_layout_count = static_cast<std::uint32_t>(layouts.size()),
		};
		{
			// Quad
			pipeline_properties.framebuffer = quad_framebuffer;
			pipeline_cache.put(pipeline_properties);
		}
		{
			// Line
			pipeline_properties.framebuffer = geometry_framebuffer;
			pipeline_properties.pipeline_key = "line";
			pipeline_properties.vertex_shader_key = "line.vert";
			pipeline_properties.fragment_shader_key = "line.frag";
			pipeline_properties.line_width = 8.0f;
			pipeline_properties.polygon_mode = PolygonMode::Line;
			pipeline_properties.layout = { { ElementType::Float3, "pos" }, { ElementType::Float4, "colour" } };
			pipeline_cache.put(pipeline_properties);
		}

		batch_renderer.construct(*this, device);
	}

	Renderer::~Renderer()
	{
		const auto& vk_device = supply_cast<Vulkan::Device>(device);
		std::for_each(std::begin(layouts), std::end(layouts),
			[&vk_device](VkDescriptorSetLayout& layout) { vkDestroyDescriptorSetLayout(vk_device, layout, nullptr); });

		vkDestroyDescriptorPool(vk_device, pool, nullptr);
		descriptors.clear();
	}

	void Renderer::on_resize()
	{
		extent = swapchain.get_extent();
		geometry_framebuffer->recreate(true, extent);
		quad_framebuffer->recreate(true, extent);
		texture_cache.force_recreate(extent);
		pipeline_cache.force_recreate(extent);
	}

	void Renderer::begin_frame(Camera& camera)
	{
		// TODO: Move to some kind of scene scope?
		batch_renderer.reset();

		uniform.view = camera.get_view_matrix();
		uniform.proj = camera.get_projection_matrix();
		uniform.view_projection = camera.get_view_projection();

		auto& current_uniform = frame_ubos[swapchain.get_current_frame()];
		current_uniform->set_data<UBO>(&uniform);

		if (swapchain.needs_recreation()) {
			force_recreation();
		}
	}

	void Renderer::end_frame() { std::memset(&uniform, 0, sizeof(UBO)); }

	void Renderer::force_recreation() { on_resize(); }

	void Renderer::submit_batched_geometry(Disarray::CommandExecutor& executor)
	{
		if (batch_renderer.should_submit()) {
			batch_renderer.submit(*this, executor);
		}

		batch_renderer.reset();
	}

	void Renderer::draw_planar_geometry(Geometry geometry, const GeometryProperties& properties)
	{
		if (batch_renderer.would_be_full()) {
			add_geometry_to_batch(geometry, properties);
			on_batch_full_func(*this);
		} else {
			add_geometry_to_batch(geometry, properties);
		}
	}

	void Renderer::add_geometry_to_batch(Disarray::Geometry geometry, const Disarray::GeometryProperties& properties)
	{
		batch_renderer.create_new(geometry, properties);
		batch_renderer.submitted_geometries++;
	}

	void Renderer::flush_batch(Disarray::CommandExecutor& executor) { batch_renderer.flush(*this, executor); }

	void Renderer::FrameDescriptor::destroy(Disarray::Device& dev) { }

} // namespace Disarray::Vulkan
