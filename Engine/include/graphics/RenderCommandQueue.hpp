#pragma once

#include "Forward.hpp"
#include "core/UsageBadge.hpp"

namespace Disarray {

class RenderCommandQueue {
public:
	using RenderCommandFunction = std::function<void()>;
	explicit RenderCommandQueue(UsageBadge<Renderer>);
	~RenderCommandQueue();

	void allocate(RenderCommandFunction&& func);
	void execute();

private:
	static constexpr auto function_size = sizeof(RenderCommandFunction);
	std::array<RenderCommandFunction, 10'000 / function_size> storage_buffer;
	std::uint32_t command_count { 0 };
};

} // namespace Disarray
