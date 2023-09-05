#include "vulkan/DebugMarker.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "core/FileWatcher.hpp"
#include "core/Log.hpp"
#include "util/BitCast.hpp"

namespace Disarray::Vulkan {

static bool active = false;
static bool extension_present = false;

PFN_vkDebugMarkerSetObjectTagEXT vkDebugMarkerSetObjectTag = VK_NULL_HANDLE; // NOLINT
PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectName = VK_NULL_HANDLE; // NOLINT
PFN_vkCmdDebugMarkerBeginEXT vkCmdDebugMarkerBegin = VK_NULL_HANDLE; // NOLINT
PFN_vkCmdDebugMarkerEndEXT vkCmdDebugMarkerEnd = VK_NULL_HANDLE; // NOLINT
PFN_vkCmdDebugMarkerInsertEXT vkCmdDebugMarkerInsert = VK_NULL_HANDLE; // NOLINT

void DebugMarker::setup(VkDevice device, VkPhysicalDevice physical_device)
{
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
		vkDebugMarkerSetObjectTag = Disarray::bit_cast<PFN_vkDebugMarkerSetObjectTagEXT>(vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectTagEXT"));
		vkDebugMarkerSetObjectName
			= Disarray::bit_cast<PFN_vkDebugMarkerSetObjectNameEXT>(vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT"));
		vkCmdDebugMarkerBegin = Disarray::bit_cast<PFN_vkCmdDebugMarkerBeginEXT>(vkGetDeviceProcAddr(device, "vkCmdDebugMarkerBeginEXT"));
		vkCmdDebugMarkerEnd = Disarray::bit_cast<PFN_vkCmdDebugMarkerEndEXT>(vkGetDeviceProcAddr(device, "vkCmdDebugMarkerEndEXT"));
		vkCmdDebugMarkerInsert = Disarray::bit_cast<PFN_vkCmdDebugMarkerInsertEXT>(vkGetDeviceProcAddr(device, "vkCmdDebugMarkerInsertEXT"));
		// Set flag if at least one function pointer is present
		active = (vkDebugMarkerSetObjectName != VK_NULL_HANDLE);

		DISARRAY_LOG_ERROR("DebugMarker", "Info: {} was present, debug markers are enabled.", std::string { VK_EXT_DEBUG_MARKER_EXTENSION_NAME });
	} else {
		DISARRAY_LOG_ERROR("DebugMarker", "Warning: {} not present, debug markers are disabled.", std::string { VK_EXT_DEBUG_MARKER_EXTENSION_NAME });
		DISARRAY_LOG_ERROR("DebugMarker", "{}", "Try running from inside a Vulkan graphics debugger (e.g. RenderDoc)");
	}
}

void DebugMarker::set_object_name(VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT object_type, const char* name)
{
	if (active) {
		VkDebugMarkerObjectNameInfoEXT name_info {};
		name_info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
		name_info.objectType = object_type;
		name_info.object = object;
		name_info.pObjectName = name;
		vkDebugMarkerSetObjectName(device, &name_info);
	}
}

void DebugMarker::set_object_tag(
	VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT object_tag, uint64_t name, size_t tag_size, const void* tag)
{
	if (active) {
		VkDebugMarkerObjectTagInfoEXT tag_info {};
		tag_info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT;
		tag_info.objectType = object_tag;
		tag_info.object = object;
		tag_info.tagName = name;
		tag_info.tagSize = tag_size;
		tag_info.pTag = tag;
		vkDebugMarkerSetObjectTag(device, &tag_info);
	}
}

void DebugMarker::begin_region(VkCommandBuffer cmdbuffer, const char* marker_name, glm::vec4 color)
{
	if (active) {
		VkDebugMarkerMarkerInfoEXT marker_info {};
		marker_info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
		std::memcpy(marker_info.color, glm::value_ptr(color), sizeof(float) * 4);
		marker_info.pMarkerName = marker_name;
		vkCmdDebugMarkerBegin(cmdbuffer, &marker_info);
	}
}

void DebugMarker::insert(VkCommandBuffer cmdbuffer, std::string marker_name, glm::vec4 color)
{
	if (active) {
		VkDebugMarkerMarkerInfoEXT marker_info {};
		marker_info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
		memcpy(marker_info.color, &color[0], sizeof(float) * 4);
		marker_info.pMarkerName = marker_name.c_str();
		vkCmdDebugMarkerInsert(cmdbuffer, &marker_info);
	}
}

void DebugMarker::end_region(VkCommandBuffer buffer)
{
	if (active) {
		vkCmdDebugMarkerEnd(buffer);
	}
}

} // namespace Disarray::Vulkan
