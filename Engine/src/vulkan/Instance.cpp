#include "DisarrayPCH.hpp"

#include "vulkan/Instance.hpp"

#include <GLFW/glfw3.h>

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "core/Log.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "util/BitCast.hpp"
#include "vulkan/Config.hpp"
#include "vulkan/QueueFamilyIndex.hpp"
#include "vulkan/Verify.hpp"
#include "vulkan/exceptions/VulkanExceptions.hpp"

std::vector<const char*> get_required_extensions()
{
	uint32_t ext_count = 0;
	const char** glfw_exts;
	glfw_exts = glfwGetRequiredInstanceExtensions(&ext_count);

	std::vector<const char*> extensions(glfw_exts, glfw_exts + ext_count);

	if constexpr (Disarray::Vulkan::Config::use_validation_layers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

#ifdef DISARRAY_MACOS
	extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

	return extensions;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type,
	const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data)
{
	switch (severity) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		Disarray::Log::debug("Validation", "Validation layer: {}", std::string(callback_data->pMessage));
		return VK_FALSE;
	default:
		Disarray::Log::error("Validation", "Validation layer: {}", std::string(callback_data->pMessage));
		return VK_FALSE;
	}
}

void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info)
{
	create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	create_info.pfnUserCallback = debug_callback;
}

VkResult create_debug_messenger_ext(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info,
	const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debug_messenger)
{
	auto func = Disarray::bit_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
	if (func != nullptr) {
		return func(instance, create_info, allocator, debug_messenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void destroy_debug_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* allocator)
{
	auto func = Disarray::bit_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
	if (func != nullptr) {
		func(instance, debug_messenger, allocator);
	}
}

namespace Disarray::Vulkan {

Instance::Instance(const std::vector<const char*>& supported_layers)
	: requested_layers(supported_layers)
{
	const auto check_support = check_validation_layer_support();
	Log::info("Instance - Validation Layers", "Requested?: {}. Supported?: {}", Config::use_validation_layers, check_support);
	if (Config::use_validation_layers && !check_support) {
		throw CouldNotCreateValidationLayersException("Could not configure validation layers, and it was asked for.");
	}

	VkApplicationInfo app_info {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Disarray";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "Disarray Engine";
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo create_info {};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;

	const auto exts = get_required_extensions();
	create_info.enabledExtensionCount = static_cast<std::uint32_t>(exts.size());
	create_info.ppEnabledExtensionNames = exts.data();

	VkDebugUtilsMessengerCreateInfoEXT debug_create_info {};
	if constexpr (Config::use_validation_layers) {
		create_info.enabledLayerCount = static_cast<std::uint32_t>(requested_layers.size());
		create_info.ppEnabledLayerNames = requested_layers.data();

		populate_debug_messenger_create_info(debug_create_info);
		create_info.pNext = &debug_create_info;
	} else {
		create_info.enabledLayerCount = 0;
	}

#ifdef DISARRAY_MACOS
	create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

	verify(vkCreateInstance(&create_info, nullptr, &instance));

	setup_debug_messenger();

	Log::debug("Vulkan Instance", "{}", "Instance created!");
}

Instance::~Instance()
{
	if (Config::use_validation_layers) {
		destroy_debug_messenger_ext(instance, debug_messenger, nullptr);
	}
	vkDestroyInstance(instance, nullptr);
	Log::debug("Vulkan Instance", "{}", "Instance destroyed!");
}

void Instance::setup_debug_messenger()
{
	if (!Config::use_validation_layers)
		return;

	VkDebugUtilsMessengerCreateInfoEXT create_info;
	populate_debug_messenger_create_info(create_info);

	verify(create_debug_messenger_ext(instance, &create_info, nullptr, &debug_messenger));
}

bool Instance::check_validation_layer_support() const
{
	uint32_t layer_count;
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

	std::vector<VkLayerProperties> available_layers(layer_count);
	vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

	for (const char* layer_name : requested_layers) {
		bool layer_found = false;

		for (const auto& layer_properties : available_layers) {
			if (std::strcmp(layer_name, layer_properties.layerName) == 0) {
				layer_found = true;
				break;
			}
		}

		if (!layer_found) {
			Log::debug("Instance", "Layer was not found");
			return false;
		}
	}

	return true;
}

} // namespace Disarray::Vulkan
