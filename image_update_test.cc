#include <vulkan/vulkan.h>
#include "vk_context.h"
#include <assert.h>

void
ImageUpdateTest(VkContext& context)
{
	VkFormat vkformat = VK_FORMAT_BC2_SRGB_BLOCK;
	VkExtent3D extents;
	extents.width = 640;
	extents.height = 480;
	extents.depth = 1;
	uint32_t queues[] = { 0 };
	VkImageCreateInfo info =
	{
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		NULL,
		0,
		VK_IMAGE_TYPE_2D,
		vkformat,
		extents,
		8,
		1,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_SHARING_MODE_CONCURRENT,
		1,
		queues,
		VK_IMAGE_LAYOUT_GENERAL
	};
	VkImage img;
	VkResult stat = vkCreateImage(VkContext::dev, &info, NULL, &img);
	assert(stat == VK_SUCCESS);

	VkDeviceMemory mem;
	uint32_t alignedSize;
	context.AllocateImageMemory(img, mem, VK_MEMORY_PROPERTY_HOST_CACHED_BIT, alignedSize);
	vkBindImageMemory(VkContext::dev, img, mem, 0);

	VkImageSubresource subres;
	subres.arrayLayer = 0;
	subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_METADATA_BIT;
	subres.mipLevel = 0;
	VkSubresourceLayout layout;
	vkGetImageSubresourceLayout(VkContext::dev, img, &subres, &layout);
}