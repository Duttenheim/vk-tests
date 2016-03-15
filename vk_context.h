#pragma once
#include <vulkan/vulkan.h>
struct VkContext
{
	static VkDevice dev;
	static VkDescriptorPool descPool;
	static VkQueue displayQueue;
	static VkQueue computeQueue;
	static VkQueue transferQueue;
	static VkInstance instance;
	static VkPhysicalDevice physicalDev;
	static VkPipelineCache cache;
	static VkCommandBuffer mainCmdGfxBuffer;
	static VkCommandBuffer mainCmdCmpBuffer;
	static VkCommandBuffer mainCmdTransBuffer;

	static VkSurfaceKHR surface;

	enum CmdCreationUsage
	{
		Persistent,
		Transient,

		NumCmdCreationUsages
	};

	static VkCommandPool cmdGfxPool[NumCmdCreationUsages];
	static VkCommandPool cmdCmpPool[NumCmdCreationUsages];
	static VkCommandPool cmdTransPool[NumCmdCreationUsages];

	const uint32_t VkPoolMaxSets = 65535;
	const uint32_t VkPoolSetSize = 64;

	VkPhysicalDevice devices[64];

	VkExtensionProperties physicalExtensions[64];

	uint32_t usedPhysicalExtensions;
	const char* deviceExtensionStrings[64];

	uint32_t usedExtensions;
	const char* extensions[64];

	uint32_t numQueues;
	VkQueueFamilyProperties queuesProps[64];

	VkPhysicalDeviceMemoryProperties memoryProps;
	uint32_t renderQueueIdx;
	uint32_t computeQueueIdx;
	uint32_t transferQueueIdx;

	// stuff used for the swap chain
	VkFormat format;
	VkColorSpaceKHR colorSpace;
	VkSwapchainKHR swapchain;

	/// setup context
	void CreateVulkanContext();
};