#pragma once
#ifdef __WIN32__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vulkan/vulkan.h>
struct Window
{
	HWND wnd;
	HINSTANCE inst;
};

void Win32OpenWindow(Window& wnd, int width, int height);
inline void OpenWindow(Window& wnd, int width, int height) { Win32OpenWindow(wnd, width, height); }
static const char* requiredOSExtensions[] = { "VK_KHR_surface", "VK_KHR_win32_surface" };
const uint32_t requiredOSExtensionsNum = 2;
#else
void X11OpenWindow(Window& wnd, int width, int height);
inline void OpenWindow(Window& wnd, int width, int height) { X11OpenWindow(wnd, width, height); }
static const char* requiredOSExtensions[] = { "VK_KHR_surface", "VK_KHR_xcb_surface" };
const uint32_t requiredOSExtensionsNum = 2;
#endif