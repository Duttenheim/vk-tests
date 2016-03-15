#include "window.h"
#include "vk_context.h"
#include <assert.h>

#ifdef __WIN32__


typedef VkFlags VkWin32SurfaceCreateFlagsKHR;

typedef struct VkWin32SurfaceCreateInfoKHR
{
	VkStructureType                 sType;
	const void*                     pNext;
	VkWin32SurfaceCreateFlagsKHR    flags;
	HINSTANCE                       hinstance;
	HWND                            hwnd;
} VkWin32SurfaceCreateInfoKHR;

typedef VkResult(APIENTRY *PFN_vkCreateWin32SurfaceKHR)(VkInstance, const VkWin32SurfaceCreateInfoKHR*, const VkAllocationCallbacks*, VkSurfaceKHR*);
typedef VkBool32(APIENTRY *PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)(VkPhysicalDevice, uint32_t);

void
Win32OpenWindow(Window& wnd, int width, int height)
{

	CreateWindowExW(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
		L"VKTEST",
		L"Vulkan test window",
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		0, 0,
		width, height,
		NULL, // No parent window
		NULL, // No window menu
		GetModuleHandleW(NULL),
		wnd.wnd); // Pass object to WM_CREATE

	VkResult err;
	VkWin32SurfaceCreateInfoKHR sci;
	PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;

	vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(VkContext::instance, "vkCreateWin32SurfaceKHR");

	memset(&sci, 0, sizeof(sci));
	sci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	sci.hinstance = GetModuleHandle(NULL);
	sci.hwnd = wnd.wnd;
	err = vkCreateWin32SurfaceKHR(VkContext::instance, &sci, NULL, &VkContext::surface);
	assert(err == VK_SUCCESS);
}
#else
void
X11OpenWindow(Window& wnd, int width, int height)
{

}
#endif