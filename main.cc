#include "vk_context.h"
#include "window.h"
#include "vk_context.h"
#include <iostream>
extern void OpenWindow(Window& wnd, int width, int height);
extern void ImageUpdateTest(VkContext& context);
extern void ValidationStandardLayerTest(VkContext& context);
extern void DescriptorPoolTest(VkContext& context);

#ifdef __WIN32__
#include <windows.h>
#define MainFunc int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
#define MainFunc int __cdecl main()
#endif
MainFunc
{
	Window window;
	OpenWindow(window, 640, 480);

	VkContext context;
	context.CreateVulkanContext(window);

	//ImageUpdateTest(context);
	ValidationStandardLayerTest(context);
	while (!window.close) 
	{
		HandleEvent(window);
	}
	return 0;
}