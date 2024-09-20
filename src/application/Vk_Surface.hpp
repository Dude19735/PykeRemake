#pragma once

#include <array>

#include "../Defines.h"
#include "Vk_Instance.hpp"

namespace VK5 {

	class Vk_Surface {
	private:
		Vk_Instance* _instance;
		VkSurfaceKHR _surface;
		std::string _title;

	public:
		Vk_Surface(
			LWWS::LWWS_Window* lwwsWindow,
			Vk_Instance* instance, 
			std::string title, 
			int width, 
			int height,
			const UT::Ut_RGBColor& bgColor,
			bool resizable,
            bool disableMousePointerOnHover=false,
            int hoverTimeoutMS=500
			) 
			:
			_instance(instance),
			_surface(nullptr),
			_title(title)
			{
			UT::Ut_Logger::Log(typeid(this), UT::GlobalCasters::castConstructorTitle("Create Surface: ") + _title);
			Vk_CheckVkResult(typeid(this), createVulkanWindowSurface(lwwsWindow, _instance->vk_instance(), nullptr), "Failed to create Vulkan surface!");
		}

		~Vk_Surface()
		{
			UT::Ut_Logger::Log(typeid(this), UT::GlobalCasters::castDestructorTitle(std::string("Destroy Surface: ") + _title));
			vkDestroySurfaceKHR(_instance->vk_instance(), _surface, nullptr);
		}

		const VkSurfaceKHR vk_surface() const {
			return _surface;
		}

	private:
#if defined(VK_USE_PLATFORM_WIN32_KHR)
		/**
		 * TODO: include multi-viewport support here.
		 */
		// VkResult createVulkanWindowSurface(VkInstance instance, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface)
        // {
        //     VkResult err;
        //     VkWin32SurfaceCreateInfoKHR sci;
        //     PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;

        //     vkCreateWin32SurfaceKHR = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR"));
        //     if (!vkCreateWin32SurfaceKHR) return VK_ERROR_EXTENSION_NOT_PRESENT;

		// 	LWWS::LWWS_Window_Win* window = reinterpret_cast<LWWS::LWWS_Window_Win*>(_window.get());
		// 	HWND hWnd;
		// 	HINSTANCE hInstance;
		// 	window->getWin32WindowDescriptors(hWnd, hInstance);

        //     memset(&sci, 0, sizeof(sci));
        //     sci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        //     sci.hinstance = hInstance;
        //     sci.hwnd = hWnd;

        //     err = vkCreateWin32SurfaceKHR(instance, &sci, allocator, surface);

        //     return err;
        // }
#elif defined(VK_USE_PLATFORM_XCB_KHR)
		/**
		 * TODO: include multi-viewport support here. Currently it's not clear how the
		 * XCB screenId can be used for this.
		 */
		// VkResult createVulkanWindowSurface(VkInstance instance, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface){
			// VkResult err;
			// VkXcbSurfaceCreateInfoKHR sci;
			// PFN_vkCreateXcbSurfaceKHR vkCreateXcbSurfaceKHR;

			// LWWS::LWWS_Window_X11* window = reinterpret_cast<LWWS::LWWS_Window_X11*>(_window.get());
			// xcb_window_t screenId;
			// Display* display;
			// window->getX11XcbWindowDescriptors(display, screenId);

			// xcb_connection_t* connection = XGetXCBConnection(display);
			// if (!connection)
			// {
			// 	return VK_ERROR_EXTENSION_NOT_PRESENT;
			// }

			// vkCreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateXcbSurfaceKHR");
			// if (!vkCreateXcbSurfaceKHR)
			// {
			// 	return VK_ERROR_EXTENSION_NOT_PRESENT;
			// }

			// memset(&sci, 0, sizeof(sci));
			// sci.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
			// sci.connection = connection;
			// sci.window = screenId;

			// err = vkCreateXcbSurfaceKHR(instance, &sci, allocator, surface);
			// return err;
		// }
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
		VkResult createVulkanWindowSurface(LWWS::LWWS_Window* window, VkInstance instance, const VkAllocationCallbacks* allocator){
			VkResult err;
			VkXlibSurfaceCreateInfoKHR sci;
			PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHR;

			vkCreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateXlibSurfaceKHR");
			if (!vkCreateXlibSurfaceKHR)
			{
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}

			LWWS::LWWS_Window_X11* lwwsWindow = reinterpret_cast<LWWS::LWWS_Window_X11*>(window);

			const auto& viewports = lwwsWindow->viewports();

			Window x11Window;
			Display* x11Display;
			lwwsWindow->getX11XlibWindowDescriptors(x11Display, 0, x11Window);

			memset(&sci, 0, sizeof(sci));
			sci.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
			sci.dpy = x11Display;
			sci.window = x11Window;

			err = vkCreateXlibSurfaceKHR(instance, &sci, allocator, &_surface);

			return err;
		}
#else
		VkResult createVulkanWindowSurface(VkInstance instance, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface){
			UT::Ut_Logger::RuntimeError(typeid(this), "Unsupported platform. Can't create Vulkan surface!");
		}
#endif
	};
}
