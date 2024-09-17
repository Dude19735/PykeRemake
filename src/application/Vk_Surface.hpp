#pragma once

#include <array>

#include "../Defines.h"
#include "Vk_Instance.hpp"

namespace VK5 {

	class Vk_Surface {
	public:
		Vk_Surface(
			Vk_Instance* instance, 
			std::string title, 
			int width, 
			int height,
			const UT::Ut_RGBColor& bgColor,
			const std::unordered_map<LWWS::TViewportId, LWWS::LWWS_Viewport>& viewports,
			bool resizable,
            bool disableMousePointerOnHover=false,
            int hoverTimeoutMS=500
			) 
			:
			_window(nullptr), 
			_instance(instance), 
			_reason(""),
			_title(title),
			_windowWidth(width),
			_windowHeight(height),
			_resizable(resizable),
			_disableMousePointerOnHover(disableMousePointerOnHover),
			_hoverTimeoutMS(hoverTimeoutMS),
			_surface({})
			{
			UT::Ut_Logger::Log(typeid(this), UT::GlobalCasters::castConstructorTitle("Create Surface"));
#if defined(VK_USE_PLATFORM_WIN32_KHR)
			UT::Ut_Logger::RuntimeError(typeid(this), "Surface type VK_USE_PLATFORM_WIN32_KHR is not implemented yet!");
			// _window = std::make_unique<LWWS::LWWS_Window_Win>(_title, _windowWidth, _windowHeight, _resizable, _disableMousePointerOnHover, _hoverTimeoutMS, "DesktopApp2", false);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
			UT::Ut_Logger::RuntimeError(typeid(this), "Surface type VK_USE_PLATFORM_XCB_KHR is not implemented yet!");
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
			_window = std::make_unique<LWWS::LWWS_Window_X11>(_title, _windowWidth, _windowHeight, bgColor, viewports, _resizable, _disableMousePointerOnHover, _hoverTimeoutMS, false);
#endif
			Vk_CheckVkResult(typeid(this), createVulkanWindowSurface(_instance->vk_instance(), nullptr), "Failed to create Vulkan surface!");
		}

		~Vk_Surface()
		{
			UT::Ut_Logger::Log(typeid(this), UT::GlobalCasters::castDestructorTitle(
				std::string("Destroy Surface") + (_reason.compare("") == 0 ? "" : std::string(" (") + _reason + std::string(")"))
			));
			for(auto& s : _surface){
				vkDestroySurfaceKHR(_instance->vk_instance(), s.second, nullptr);
			}
		}

		const VkSurfaceKHR vk_surface(LWWS::TViewportId id) const {
			if(!_surface.contains(id)){
				UT::Ut_Logger::RuntimeError(typeid(this), _window->genViewportErrMsg(id));
			}

			return _surface.at(id);
		}

		/**
		 * TODO: is this one needed?
		 */
		const VkExtent2D vk_canvasSize() const {
			int w,h;
			_window->canvasSize(w, h);
			return  VkExtent2D{
				.width=static_cast<uint32_t>(w),
				.height=static_cast<uint32_t>(h)
			};
		}

		/**
		 * TODO: is this one needed?
		 */
		const VkExtent2D vk_canvasOriginalSize() const {
			int w,h;
			_window->canvasInitSize(w, h);
			return  VkExtent2D{
				.width=static_cast<uint32_t>(w),
				.height=static_cast<uint32_t>(h)
			};
		}

		/**
		 * TODO: is this one needed?
		 */
		const VkExtent2D vk_frameSize() const {
			int w,h;
			_window->frameSize(w, h);
			return  VkExtent2D{
				.width=static_cast<uint32_t>(w),
				.height=static_cast<uint32_t>(h)
			};
		}

		/**
		 * TODO: is this one needed?
		 * => Vulkan may get this from device support struct directly
		 */
		const VkExtent2D vk_viewportSize(LWWS::TViewportId viewportId) const {
			int w,h;
			_window->viewportSize(viewportId, w, h);
			return  VkExtent2D{
				.width=static_cast<uint32_t>(w),
				.height=static_cast<uint32_t>(h)
			};
		}

		LWWS::LWWS_Window* vk_lwws_window() const {
			return _window.get();
		}

	private:
		// GLFWwindow* _window;
		std::unique_ptr<LWWS::LWWS_Window> _window;
		Vk_Instance* _instance;
		std::string _reason;

		std::string _title;
		int _windowWidth;
		int _windowHeight;
		bool _resizable;
		bool _disableMousePointerOnHover;
		int _hoverTimeoutMS;

		std::unordered_map<LWWS::TViewportId, VkSurfaceKHR> _surface;

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
		VkResult createVulkanWindowSurface(VkInstance instance, const VkAllocationCallbacks* allocator){
			VkResult err;
			VkXlibSurfaceCreateInfoKHR sci;
			PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHR;

			vkCreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateXlibSurfaceKHR");
			if (!vkCreateXlibSurfaceKHR)
			{
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}

			LWWS::LWWS_Window_X11* lwwsWindow = reinterpret_cast<LWWS::LWWS_Window_X11*>(_window.get());

			const auto& viewports = lwwsWindow->viewports();

			for(const auto vp : viewports){
				Window window;
				Display* display;
				lwwsWindow->getX11XlibWindowDescriptors(display, 0, window);

				memset(&sci, 0, sizeof(sci));
				sci.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
				sci.dpy = display;
				sci.window = window;

				_surface.insert({vp.first, nullptr});
				err = vkCreateXlibSurfaceKHR(instance, &sci, allocator, &_surface.at(vp.first));

				if(err != VK_SUCCESS) return err;
			}
			return err;
		}
#else
		VkResult createVulkanWindowSurface(VkInstance instance, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface){
			UT::Ut_Logger::RuntimeError(typeid(this), "Unsupported platform. Can't create Vulkan surface!");
		}
#endif
	};
}
