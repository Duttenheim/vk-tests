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


static LRESULT __stdcall WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	Window* window = (Window*)GetWindowLongPtrW(hwnd, 0);
	switch (msg)
	{
	case WM_SIZING: return TRUE;
	case WM_NCCREATE:
	{
		CREATESTRUCTW* cs = (CREATESTRUCTW*)lparam;
		SetWindowLongPtrW(hwnd, 0, (LONG_PTR)cs->lpCreateParams);
		break;
	}
	case WM_CLOSE:
		CloseWindow(hwnd);
		window->close = true;
		break;
	case WM_SETFOCUS:
		return 0;
	case WM_ERASEBKGND:
		return TRUE;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}


void
Win32OpenWindow(Window& wnd, int width, int height)
{
	WNDCLASSEXW wc;

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbWndExtra = sizeof(void*) + sizeof(int); // Make room for one pointer
	wc.hInstance = GetModuleHandleW(NULL);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.lpszClassName = L"VKTEST";
	assert(RegisterClassExW(&wc) != NULL);

	wnd.wnd = CreateWindowExW(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
		L"VKTEST",
		L"Vulkan test window",
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		width/2, height/2,
		width, height,
		NULL, // No parent window
		NULL, // No window menu
		GetModuleHandleW(NULL),
		&wnd); // Pass object to WM_CREATE

	ShowWindow(wnd.wnd, 1);
	UpdateWindow(wnd.wnd);
	SetWindowPos(wnd.wnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);


}

void
Win32CreateSurface(Window& wnd)
{
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

bool
Win32HandleEvent(Window& wnd)
{
	MSG msg;
	while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT) return false;
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return true;
}

#else
void
X11OpenWindow(Window& wnd, int width, int height)
{

}
#endif