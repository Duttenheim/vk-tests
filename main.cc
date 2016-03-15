#include "vk_context.h"
#include "window.h"
#include "vk_context.h"
#include <iostream>
extern void OpenWindow(Window& wnd, int width, int height);
extern void ImageUpdateTest();
extern void ValidationStandardLayerTest();
extern void DescriptorPoolTest();
void 
main()
{
	VkContext context;
	context.CreateVulkanContext();

	Window window;
	OpenWindow(window, 640, 480);

	std::cin.get();
}