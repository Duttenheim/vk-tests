#pragma once
#ifdef __WIN32__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <vulkan/vulkan.h>
struct Window
{
	HWND wnd;
	HINSTANCE inst;
	bool close;

	Window() : close(false) {};
};

void Win32OpenWindow(Window& wnd, int width, int height);
bool Win32HandleEvent(Window& wnd);
void Win32CreateSurface(Window& wnd);

inline void OpenWindow(Window& wnd, int width, int height) { Win32OpenWindow(wnd, width, height); }
inline bool HandleEvent(Window& wnd) { return Win32HandleEvent(wnd); }
inline void CreateSurface(Window& wnd) { return Win32CreateSurface(wnd); }

static const char* requiredOSExtensions[] = { "VK_KHR_surface", "VK_KHR_win32_surface" };
const uint32_t requiredOSExtensionsNum = 2;
#else
void X11OpenWindow(Window& wnd, int width, int height);
bool X11HandleEvent(Window& wnd);
void X11CreateSurfaceWindow& wnd);

inline void OpenWindow(Window& wnd, int width, int height) { X11OpenWindow(wnd, width, height); }
inline bool HandleEvent(Window& wnd) { return X11HandleEvent(wnd); }
inline void CreateSurface(Window& wnd) { return X11CreateSurface(wnd); }

static const char* requiredOSExtensions[] = { "VK_KHR_surface", "VK_KHR_xcb_surface" };
const uint32_t requiredOSExtensionsNum = 2;
#endif