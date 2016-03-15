#include "vk_context.h"
#include "window.h"
#include <assert.h>

VkDevice VkContext::dev;
VkDescriptorPool VkContext::descPool;
VkQueue VkContext::displayQueue;
VkQueue VkContext::computeQueue;
VkQueue VkContext::transferQueue;
VkInstance VkContext::instance;
VkPhysicalDevice VkContext::physicalDev;
VkPipelineCache VkContext::cache;

VkCommandPool VkContext::cmdCmpPool[2];
VkCommandPool VkContext::cmdTransPool[2];
VkCommandPool VkContext::cmdGfxPool[2];
VkCommandBuffer VkContext::mainCmdGfxBuffer;
VkCommandBuffer VkContext::mainCmdCmpBuffer;
VkCommandBuffer VkContext::mainCmdTransBuffer;
VkSurfaceKHR VkContext::surface;

void
VkContext::CreateVulkanContext()
{
	VkResult res;

	// setup application
	VkApplicationInfo appInfo =
	{
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		NULL,
		"Vk testing",
		1,					// application version
		"Nebula Trifid",	// engine name
		1,					// engine version
		VK_API_VERSION		// API version
	};

	this->usedExtensions = 0;
	const char** requiredExtensions = requiredOSExtensions;
	uint32_t i;
	for (i = 0; i < (uint32_t)requiredOSExtensionsNum; i++)
	{
		this->extensions[this->usedExtensions++] = requiredExtensions[i];
	}

	const char* layers[] = { "VK_LAYER_LUNARG_mem_tracker", "VK_LAYER_LUNARG_object_tracker", "VK_LAYER_LUNARG_device_limits", "VK_LAYER_LUNARG_threading", "VK_LAYER_LUNARG_param_checker", "VK_LAYER_LUNARG_draw_state" };

#if USE_VALIDATION_LAYERS
	this->extensions[this->usedExtensions++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
	const int numLayers = 5;
#else
	const int numLayers = 0;
#endif

	// setup instance
	VkInstanceCreateInfo instanceInfo =
	{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,		// type of struct
		NULL,										// pointer to next
		0,											// flags
		&appInfo,									// application
		numLayers,
		layers,
		this->usedExtensions,
		this->extensions
	};

	// create instance
	res = vkCreateInstance(&instanceInfo, NULL, &this->instance);
	if (res == VK_ERROR_INCOMPATIBLE_DRIVER)
	{
		assert(false && "VkRenderDevice::OpenVulkanContext(): Your GPU driver is not compatible with Vulkan.\n");
	}
	else if (res == VK_ERROR_EXTENSION_NOT_PRESENT)
	{
		assert(false && "VkRenderDevice::OpenVulkanContext(): Vulkan extension failed to load.\n");
	}
	else if (res == VK_ERROR_LAYER_NOT_PRESENT)
	{
		assert(false && "VkRenderDevice::OpenVulkanContext(): Vulkan layer failed to load.\n");
	}
	assert(res == VK_SUCCESS);

	// retrieve available GPUs
	uint32_t gpuCount;
	res = vkEnumeratePhysicalDevices(this->instance, &gpuCount, NULL);
	assert(res == VK_SUCCESS);

	if (gpuCount > 0)
	{
		res = vkEnumeratePhysicalDevices(this->instance, &gpuCount, this->devices);
		assert(res == VK_SUCCESS);

		// hmm, this is ugly, perhaps implement a way to get a proper device
		this->physicalDev = devices[0];
	}
	else
	{
		assert(false && "VkRenderDevice::SetupAdapter(): No GPU available.\n");
	}

	res = vkEnumerateDeviceExtensionProperties(this->physicalDev, NULL, &this->usedPhysicalExtensions, NULL);
	assert(res == VK_SUCCESS);

	if (this->usedPhysicalExtensions > 0)
	{
		res = vkEnumerateDeviceExtensionProperties(this->physicalDev, NULL, &this->usedPhysicalExtensions, this->physicalExtensions);

		uint32_t i;
		for (i = 0; i < this->usedPhysicalExtensions; i++)
		{
			this->deviceExtensionStrings[i] = this->physicalExtensions[i].extensionName;
		}
	}

	// get number of queues
	vkGetPhysicalDeviceQueueFamilyProperties(this->physicalDev, &this->numQueues, NULL);
	assert(this->numQueues > 0);

	// now get queues from device
	vkGetPhysicalDeviceQueueFamilyProperties(this->physicalDev, &this->numQueues, this->queuesProps);
	vkGetPhysicalDeviceMemoryProperties(this->physicalDev, &this->memoryProps);
}