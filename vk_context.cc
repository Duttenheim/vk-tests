#include "vk_context.h"
#include "window.h"
#include <assert.h>
#include <vector>

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
VkContext::CreateVulkanContext(Window& wnd)
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

	const char* layers[] = { "VK_LAYER_LUNARG_standard_validation" "VK_LAYER_LUNARG_mem_tracker", "VK_LAYER_LUNARG_object_tracker", "VK_LAYER_LUNARG_device_limits", "VK_LAYER_LUNARG_threading", "VK_LAYER_LUNARG_param_checker", "VK_LAYER_LUNARG_draw_state" };

#if 1
	this->extensions[this->usedExtensions++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
	const int numLayers = 1;
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

	CreateSurface(wnd);

	VkBool32* canPresent = new VkBool32[numQueues];
	for (i = 0; i < numQueues; i++)
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(VkContext::physicalDev, i, VkContext::surface, &canPresent[i]);
	}

	uint32_t gfxIdx = UINT32_MAX;
	uint32_t computeIdx = UINT32_MAX;
	uint32_t transferIdx = UINT32_MAX;
	uint32_t queueIdx = UINT32_MAX;
	this->renderQueueIdx = UINT32_MAX;
	this->computeQueueIdx = UINT32_MAX;
	this->transferQueueIdx = UINT32_MAX;

	// create three queues for each family
	std::vector<uint32_t> indexMap;
	indexMap.resize(numQueues);
	memset(&indexMap[0], 0, indexMap.size() * sizeof(uint32_t));
	for (i = 0; i < numQueues; i++)
	{
		if (this->queuesProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			uint32_t j;
			for (j = 0; j < numQueues; j++)
			{
				if (canPresent[i] == VK_TRUE)
				{
					this->renderQueueIdx = j;
					gfxIdx = indexMap[i];
					indexMap[i]++;
					break;
				}
			}
		}

		// also setup compute and transfer queues
		if (this->queuesProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT && this->computeQueueIdx == UINT32_MAX)
		{
			if (this->queuesProps[i].queueCount == indexMap[i]) continue;
			computeIdx = i;
			this->computeQueueIdx = indexMap[i];
			indexMap[i]++;
		}
		if (this->queuesProps[i].queueFlags & VK_QUEUE_TRANSFER_BIT && this->transferQueueIdx == UINT32_MAX)
		{
			if (this->queuesProps[i].queueCount == indexMap[i]) continue;
			transferIdx = i;
			this->transferQueueIdx = indexMap[i];
			indexMap[i]++;
		}
	}

	if (this->renderQueueIdx == UINT32_MAX || gfxIdx == UINT32_MAX) assert(false && "VkContext: Could not find a queue that supported screen present and graphics.\n");

	// delete array of present flags
	delete [] canPresent;

	std::vector<std::vector<float>> prios;
	std::vector<VkDeviceQueueCreateInfo> queueInfos;
	prios.resize(numQueues);

	for (i = 0; i < numQueues; i++)
	{
		if (indexMap[i] == 0) continue;
		prios[i].resize(indexMap[i]);
		memset(&prios[0], 0, prios[i].size() * sizeof(float));
		queueInfos.push_back(
		{
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			NULL,
			0,
			i,
			indexMap[i],
			&prios[i][0]
		});
	}

	// get physical device features
	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures(this->physicalDev, &features);

	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(this->physicalDev, &props);

	VkDeviceCreateInfo deviceInfo =
	{
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		NULL,
		0,
		queueInfos.size(),
		&queueInfos[0],
		numLayers,
		layers,
		this->usedPhysicalExtensions,
		this->deviceExtensionStrings,
		&features
	};

	// create device
	res = vkCreateDevice(this->physicalDev, &deviceInfo, NULL, &this->dev);
	assert(res == VK_SUCCESS);

	vkGetDeviceQueue(VkContext::dev, gfxIdx, this->renderQueueIdx, &VkContext::displayQueue);
	vkGetDeviceQueue(VkContext::dev, computeIdx, this->computeQueueIdx, &VkContext::computeQueue);
	vkGetDeviceQueue(VkContext::dev, transferIdx, this->transferQueueIdx, &VkContext::transferQueue);

	// find available surface formats
	uint32_t numFormats;
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(VkContext::physicalDev, VkContext::surface, &numFormats, NULL);
	assert(res == VK_SUCCESS);

	VkSurfaceFormatKHR* formats = new VkSurfaceFormatKHR[numFormats];
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(VkContext::physicalDev, VkContext::surface, &numFormats, formats);
	assert(res == VK_SUCCESS);
	if (numFormats == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
	{
		// is this really the goto format?
		// perhaps assuming sRGB is a bit risky when we can't even get a format to begin with
		this->format = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else
	{
		this->format = formats[0].format;
	}
	this->colorSpace = formats[0].colorSpace;

	// get surface capabilities
	VkSurfaceCapabilitiesKHR surfCaps;
	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->physicalDev, VkContext::surface, &surfCaps);
	assert(res == VK_SUCCESS);

	uint32_t numPresentModes;
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(this->physicalDev, VkContext::surface, &numPresentModes, NULL);
	assert(res == VK_SUCCESS);

	// get present modes
	VkPresentModeKHR* presentModes = new VkPresentModeKHR[numPresentModes];
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(this->physicalDev, VkContext::surface, &numPresentModes, presentModes);
	assert(res == VK_SUCCESS);

	VkExtent2D swapchainExtent;
	if (surfCaps.currentExtent.width == -1)
	{
		swapchainExtent.width = 640;
		swapchainExtent.height = 480;
	}
	else
	{
		swapchainExtent = surfCaps.currentExtent;
	}

	// figure out the best present mode, mailo
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (i = 0; i < numPresentModes; i++)
	{
		if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
		{
			swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	// get the optimal set of swap chain images, the more the better
	uint32_t numSwapchainImages = surfCaps.minImageCount + 1;
	if ((surfCaps.maxImageCount > 0) && (numSwapchainImages > surfCaps.maxImageCount)) numSwapchainImages = surfCaps.maxImageCount;

	// create a transform
	VkSurfaceTransformFlagBitsKHR transform;
	if (surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	else																	  transform = surfCaps.currentTransform;

	VkSwapchainCreateInfoKHR swapchainInfo =
	{
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		NULL,
		0,
		VkContext::surface,
		numSwapchainImages,
		this->format,
		this->colorSpace,
		swapchainExtent,
		1,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		this->renderQueueIdx,
		NULL,
		transform,
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		swapchainPresentMode,
		true,
		NULL
	};

	// create swapchain
	res = vkCreateSwapchainKHR(this->dev, &swapchainInfo, NULL, &this->swapchain);
	assert(res == VK_SUCCESS);

	// get back buffers
	uint32_t numSwapchainBackbuffers;
	res = vkGetSwapchainImagesKHR(this->dev, this->swapchain, &numSwapchainBackbuffers, NULL);
	assert(res == VK_SUCCESS);

	this->backbuffers = new VkImage[numSwapchainBackbuffers];
	res = vkGetSwapchainImagesKHR(this->dev, this->swapchain, &numSwapchainBackbuffers, this->backbuffers);

	VkPipelineCacheCreateInfo cacheInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		NULL,
		0,
		0,
		NULL
	};

	// create cache
	res = vkCreatePipelineCache(this->dev, &cacheInfo, NULL, &this->cache);
	assert(res == VK_SUCCESS);

	VkDescriptorPoolSize sizes[11];
	VkDescriptorType types[] =
	{
		VK_DESCRIPTOR_TYPE_SAMPLER,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
		VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
		VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
	};

	for (i = 0; i < 11; i++)
	{
		sizes[i].descriptorCount = VkPoolSetSize;
		sizes[i].type = types[i];
	}

	VkDescriptorPoolCreateInfo poolInfo =
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		NULL,
		VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		VkPoolMaxSets,
		11,
		sizes
	};

	res = vkCreateDescriptorPool(this->dev, &poolInfo, NULL, &this->descPool);
	assert(res == VK_SUCCESS);
}

void
VkContext::AllocateBufferMemory(const VkBuffer& buf, VkDeviceMemory& mem, VkMemoryPropertyFlagBits flags, uint32_t& size)
{
	// now attain memory requirements so we get a properly aligned memory storage
	VkMemoryRequirements req;
	vkGetBufferMemoryRequirements(VkContext::dev, buf, &req);

	uint32_t memtype;
	VkResult err = this->GetMemoryType(req.memoryTypeBits, flags, memtype);
	assert(err == VK_SUCCESS);
	VkMemoryAllocateInfo meminfo =
	{
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		NULL,
		req.size,
		memtype
	};

	// now allocate memory
	err = vkAllocateMemory(this->dev, &meminfo, NULL, &mem);
	if (err == VK_ERROR_OUT_OF_DEVICE_MEMORY || err == VK_ERROR_OUT_OF_HOST_MEMORY)
	{
		assert(false && "Cannot allocate memory\n");
	}
	assert(err == VK_SUCCESS);
	size = (uint32_t)req.size;
}

void 
VkContext::AllocateImageMemory(const VkImage& buf, VkDeviceMemory& mem, VkMemoryPropertyFlagBits flags, uint32_t& size)
{
	// now attain memory requirements so we get a properly aligned memory storage
	VkMemoryRequirements req;
	vkGetImageMemoryRequirements(VkContext::dev, buf, &req);

	uint32_t memtype;
	VkResult err = this->GetMemoryType(req.memoryTypeBits, flags, memtype);
	assert(err == VK_SUCCESS);
	VkMemoryAllocateInfo meminfo =
	{
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		NULL,
		req.size,
		memtype
	};

	// now allocate memory
	err = vkAllocateMemory(this->dev, &meminfo, NULL, &mem);
	if (err == VK_ERROR_OUT_OF_DEVICE_MEMORY || err == VK_ERROR_OUT_OF_HOST_MEMORY)
	{
		assert(false && "Cannot allocate memory\n");
	}
	assert(err == VK_SUCCESS);
	size = (uint32_t)req.size;
}

VkResult
VkContext::GetMemoryType(uint32_t bits, VkMemoryPropertyFlags flags, uint32_t& index)
{
	for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++)
	{
		if ((bits & 1) == 1)
		{
			if ((this->memoryProps.memoryTypes[i].propertyFlags & flags) == flags)
			{
				index = i;
				return VK_SUCCESS;
			}
		}
		bits >>= 1;
	}
	return VK_ERROR_FEATURE_NOT_PRESENT;
}