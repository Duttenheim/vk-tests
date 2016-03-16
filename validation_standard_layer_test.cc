#include <vulkan/vulkan.h>
#include "vk_context.h"
#include <assert.h>
#include <vector>

void
ValidationStandardLayerTest(VkContext& context)
{
	std::vector<VkDescriptorSetLayout> layouts;

	for (uint32_t i = 0; i < 5; i++)
	{
		std::vector<VkDescriptorSetLayoutBinding> binds;
		for (uint32_t j = 0; j < 20; j++)
		{
			VkDescriptorSetLayoutBinding bind;
			bind.stageFlags = VK_SHADER_STAGE_ALL;
			bind.binding = j;
			bind.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			bind.descriptorCount = 1;
			bind.pImmutableSamplers = VK_NULL_HANDLE;
			binds.push_back(bind);
		}

		VkDescriptorSetLayoutCreateInfo info;
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.pNext = NULL;
		info.flags = 0;
		info.bindingCount = binds.size();
		info.pBindings = &binds[0];

		// create layout
		VkDescriptorSetLayout layout;
		VkResult res = vkCreateDescriptorSetLayout(VkContext::dev, &info, NULL, &layout);
		assert(res == VK_SUCCESS);
		layouts.push_back(layout);
	}

	VkPipelineLayoutCreateInfo layoutInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		NULL,
		0,
		layouts.size(),
		&layouts[0],
		0,
		NULL
	};

	// create pipeline layout, every program should inherit this one
	VkPipelineLayout playout;
	VkResult res = vkCreatePipelineLayout(VkContext::dev, &layoutInfo, NULL, &playout);
	assert(res == VK_SUCCESS);

	// allocate descriptor sets
	VkDescriptorSetAllocateInfo setInfo =
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		NULL,
		VkContext::descPool,
		layouts.size(),
		&layouts[0]
	};

	std::vector<VkDescriptorSet> descriptorSets;
	descriptorSets.resize(layouts.size());
	res = vkAllocateDescriptorSets(VkContext::dev, &setInfo, &descriptorSets[0]);
	assert(res == VK_SUCCESS);
}