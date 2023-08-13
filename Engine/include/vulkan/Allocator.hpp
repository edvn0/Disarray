#pragma once

#include "core/DataBuffer.hpp"
#include "core/Types.hpp"
#include "util/BitCast.hpp"
#include "vulkan/MemoryAllocator.hpp"

#include <cstring>
#include <string>
#include <vulkan/vulkan.h>

namespace Disarray::Vulkan {

	class Device;
	class Instance;
	class PhysicalDevice;

	enum class Usage : int {
		UNKNOWN = 0,
		GPU_ONLY = 1,
		CPU_ONLY = 2,
		CPU_TO_GPU = 3,
		GPU_TO_CPU = 4,
		CPU_COPY = 5,
		GPU_LAZILY_ALLOCATED = 6,
		AUTO = 7,
		AUTO_PREFER_DEVICE = 8,
		AUTO_PREFER_HOST = 9,
	};
	enum class Creation : std::uint32_t {
		DEDICATED_MEMORY_BIT = 0x00000001,
		NEVER_ALLOCATE_BIT = 0x00000002,
		MAPPED_BIT = 0x00000004,
		USER_DATA_COPY_STRING_BIT = 0x00000020,
		UPPER_ADDRESS_BIT = 0x00000040,
		DONT_BIND_BIT = 0x00000080,
		WITHIN_BUDGET_BIT = 0x00000100,
		CAN_ALIAS_BIT = 0x00000200,
		HOST_ACCESS_SEQUENTIAL_WRITE_BIT = 0x00000400,
		HOST_ACCESS_RANDOM_BIT = 0x00000800,
		HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT = 0x00001000,
		STRATEGY_MIN_MEMORY_BIT = 0x00010000,
		STRATEGY_MIN_TIME_BIT = 0x00020000,
		STRATEGY_MIN_OFFSET_BIT = 0x00040000,
		STRATEGY_BEST_FIT_BIT = STRATEGY_MIN_MEMORY_BIT,
		STRATEGY_FIRST_FIT_BIT = STRATEGY_MIN_TIME_BIT,
		STRATEGY_MASK = STRATEGY_MIN_MEMORY_BIT | STRATEGY_MIN_TIME_BIT | STRATEGY_MIN_OFFSET_BIT,
	};

	inline constexpr Creation operator|(Creation left, Creation right)
	{
		return static_cast<Creation>(static_cast<std::uint32_t>(left) | static_cast<std::uint32_t>(right));
	}

	inline constexpr Creation& operator|=(Creation& a, Creation b) { return a = a | b; }

	struct AllocationProperties {
		Usage usage;
		Creation creation;
	};

	class Allocator {
	public:
		Allocator(const std::string& resource_name = "Unknown Resource");
		~Allocator();

		static void initialise(Vulkan::Device&, Vulkan::PhysicalDevice&, Vulkan::Instance&);
		static void shutdown();

		template <typename T = UnkownData> T* map_memory(VmaAllocation allocation)
		{
			T* data { nullptr };
			vmaMapMemory(allocator, allocation, Disarray::bit_cast<void**>(&data));
			return data;
		}

		void unmap_memory(VmaAllocation allocation) { vmaUnmapMemory(allocator, allocation); }

		VmaAllocation allocate_buffer(VkBuffer&, VkBufferCreateInfo, const AllocationProperties&);
		VmaAllocation allocate_buffer(VkBuffer&, VmaAllocationInfo&, VkBufferCreateInfo, const AllocationProperties&);
		VmaAllocation allocate_image(VkImage&, VkImageCreateInfo, const AllocationProperties&);

		void deallocate_buffer(VmaAllocation, VkBuffer&);
		void deallocate_image(VmaAllocation, VkImage&);

	private:
		std::string resource_name {};

		inline static VmaAllocator allocator;
	};

	template <typename T> class AllocationMapper {
	public:
		AllocationMapper(Allocator& allocate, VmaAllocation alloc, const void* data, std::size_t size)
			: allocator(allocate)
			, allocation(alloc)
		{
			auto* out = allocator.map_memory<T>(allocation);
			std::memcpy(out, data, size);
		}

		AllocationMapper(Allocator& allocate, VmaAllocation alloc, const DataBuffer& data_buffer)
			: allocator(allocate)
			, allocation(alloc)
		{
			auto* out = allocator.map_memory<T>(allocation);
			std::memcpy(out, data_buffer.get_data(), data_buffer.get_size());
		}

		~AllocationMapper() { allocator.unmap_memory(allocation); }

	private:
		Allocator& allocator;
		VmaAllocation allocation;
	};

} // namespace Disarray::Vulkan
