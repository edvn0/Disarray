#include "vulkan/DebugMarker.hpp"

#include "core/Log.hpp"

#include <glm/gtc/type_ptr.hpp>

namespace Disarray::Vulkan {

	bool active = false;
	bool extension_present = false;

	PFN_vkDebugMarkerSetObjectTagEXT vkDebugMarkerSetObjectTag = VK_NULL_HANDLE; // NOLINT
	PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectName = VK_NULL_HANDLE; // NOLINT
	PFN_vkCmdDebugMarkerBeginEXT vkCmdDebugMarkerBegin = VK_NULL_HANDLE; // NOLINT
	PFN_vkCmdDebugMarkerEndEXT vkCmdDebugMarkerEnd = VK_NULL_HANDLE; // NOLINT
	PFN_vkCmdDebugMarkerInsertEXT vkCmdDebugMarkerInsert = VK_NULL_HANDLE; // NOLINT

	// Get function pointers for the debug report extensions from the device
	void DebugMarker::setup(VkDevice device, VkPhysicalDevice physical_device)
	{
		// Check if the debug marker extension is present (which is the case if run from a graphics debugger)
		uint32_t extension_count;
		vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);
		std::vector<VkExtensionProperties> extensions(extension_count);
		vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, extensions.data());
		for (const auto& extension : extensions) {
			if (strcmp(extension.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0) {
				extension_present = true;
				break;
			}
		}

		if (extension_present) {
			// The debug marker extension is not part of the core, so function pointers need to be loaded manually
			vkDebugMarkerSetObjectTag = (PFN_vkDebugMarkerSetObjectTagEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectTagEXT"); // NOLINT
			vkDebugMarkerSetObjectName = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(device,
				"vkDebugMarkerSetObjectNameEXT"); // NOLINT
			vkCmdDebugMarkerBegin = (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(device,
				"vkCmdDebugMarkerBeginEXT"); // NOLINT
			vkCmdDebugMarkerEnd = (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(device,
				"vkCmdDebugMarkerEndEXT"); // NOLINT
			vkCmdDebugMarkerInsert = (PFN_vkCmdDebugMarkerInsertEXT)vkGetDeviceProcAddr(device,
				"vkCmdDebugMarkerInsertEXT"); // NOLINT
			// Set flag if at least one function pointer is present
			active = (vkDebugMarkerSetObjectName != VK_NULL_HANDLE);

			Log::error("DebugMarker", "Info: " + std::string { VK_EXT_DEBUG_MARKER_EXTENSION_NAME } + " was present, debug markers are enabled.");
		} else {
			Log::error("DebugMarker", "Warning: " + std::string { VK_EXT_DEBUG_MARKER_EXTENSION_NAME } + " not present, debug markers are disabled.");
			Log::error("DebugMarker", "Try running from inside a Vulkan graphics debugger (e.g. RenderDoc)");
		}
	}

	// Sets the debug name of an object
	// All Objects in Vulkan are represented by their 64-bit handles which are passed into this function
	// along with the object type
	void DebugMarker::set_object_name(VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT object_type, const char* name)
	{
		// Check for valid function pointer (may not be present if not running in a debugging application)
		if (active) {
			VkDebugMarkerObjectNameInfoEXT name_info = { VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT };
			name_info.objectType = object_type;
			name_info.object = object;
			name_info.pObjectName = name;
			vkDebugMarkerSetObjectName(device, &name_info);
		}
	}

	// Set the tag for an object
	void DebugMarker::set_object_tag(
		VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT object_tag, uint64_t name, size_t tag_size, const void* tag)
	{
		// Check for valid function pointer (may not be present if not running in a debugging application)
		if (active) {
			VkDebugMarkerObjectTagInfoEXT tag_info = { VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT };
			tag_info.objectType = object_tag;
			tag_info.object = object;
			tag_info.tagName = name;
			tag_info.tagSize = tag_size;
			tag_info.pTag = tag;
			vkDebugMarkerSetObjectTag(device, &tag_info);
		}
	}

	// Start a new debug marker region
	void DebugMarker::begin_region(VkCommandBuffer cmdbuffer, const char* marker_name, glm::vec4 color)
	{
		// Check for valid function pointer (may not be present if not running in a debugging application)
		if (active) {
			VkDebugMarkerMarkerInfoEXT marker_info = {};
			marker_info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
			std::memcpy(marker_info.color, glm::value_ptr(color), sizeof(float) * 4);
			marker_info.pMarkerName = marker_name;
			vkCmdDebugMarkerBegin(cmdbuffer, &marker_info);
		}
	}

	// Insert a new debug marker into the command buffer
	void DebugMarker::insert(VkCommandBuffer cmdbuffer, std::string marker_name, glm::vec4 color)
	{
		// Check for valid function pointer (may not be present if not running in a debugging application)
		if (active) {
			VkDebugMarkerMarkerInfoEXT marker_info = {};
			marker_info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
			memcpy(marker_info.color, &color[0], sizeof(float) * 4);
			marker_info.pMarkerName = marker_name.c_str();
			vkCmdDebugMarkerInsert(cmdbuffer, &marker_info);
		}
	}

	// End the current debug marker region
	void DebugMarker::end_region(VkCommandBuffer buffer)
	{
		// Check for valid function (may not be present if not running in a debugging application)
		if (active) {
			vkCmdDebugMarkerEnd(buffer);
		}
	}

} // namespace Disarray::Vulkan
