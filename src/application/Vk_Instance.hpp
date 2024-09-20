#pragma once

#include "../Defines.h"
// #include "../UT::Ut_Logger.hpp"

namespace VK5 {
	class Vk_Instance {
	private:
		std::vector<const char*> _deviceExtensions;
		std::string _applicationName;

		VkInstance _instance;
#ifdef _DEBUG
		VkDebugUtilsMessengerEXT _debugMessenger;
#endif

	public:
		Vk_Instance(std::string applicationName) 
			:
			_applicationName(applicationName)
#ifdef _DEBUG
			,_debugMessenger(VK_NULL_HANDLE)
#endif
		{
#ifdef _DEBUG
			UT::Ut_Logger::Warn(typeid(this), UT::GlobalCasters::castHighlightYellow(std::string("Debug mode enabled")));
#endif
			initApplication();
			UT::Ut_Logger::Log(typeid(this), UT::GlobalCasters::castConstructorTitle(std::string("Create Instance: ") + _applicationName));
		}
		~Vk_Instance() {
			UT::Ut_Logger::Log(typeid(this), UT::GlobalCasters::castDestructorTitle(std::string("Destroy Instance: ") + _applicationName));
#ifdef _DEBUG
			if (_debugMessenger) {
				PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT");
				func(_instance, _debugMessenger, nullptr);
			}
#endif
			vkDestroyInstance(_instance, nullptr);
		}

		VkInstance vk_instance() const {
			return _instance;
		}

		std::string vk_applicationName() const {
			return _applicationName;
		}

	private:
		void initApplication() {
			// filling this struct is optional
			VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
			appInfo.pApplicationName = _applicationName.c_str();
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.pEngineName = "No Engine";
			appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.apiVersion = VK_API_VERSION_1_2;
			appInfo.pNext = nullptr; // may point to additional information in the future, leave it nullptr

			// this struct is not optional
			VkInstanceCreateInfo instanceCreateInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
			instanceCreateInfo.pApplicationInfo = &appInfo;

			std::vector<const char*> platformExtensions = _getRequiredExtensions();

			instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(platformExtensions.size());
			instanceCreateInfo.ppEnabledExtensionNames = platformExtensions.data();

			// validation layers
			std::vector<const char*> requiredValidationLayers = {
				//VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME
				"VK_LAYER_KHRONOS_validation"
			};

			// get available layers
			uint32_t availableLayerCount = 0;
			Vk_CheckVkResult(typeid(this), vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr), "Failed to get instance layer property count");
			std::vector<VkLayerProperties> availableLayers(availableLayerCount);
			Vk_CheckVkResult(typeid(this), vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.data()), "Failed to enumerate available layers");


			// verify that all required layers are available
			// bool success = true;
#ifdef _DEBUG
			for (uint32_t i = 0; i < static_cast<uint32_t>(requiredValidationLayers.size()); ++i) {
				bool found = false;
				for (uint32_t j = 0; j < availableLayerCount; ++j) {
					if (strcmp(requiredValidationLayers[i], availableLayers[j].layerName) == 0) {
						found = true;
						break;
					}
				}

				if (!found) {
					UT::Ut_Logger::RuntimeError(typeid(this), "Required validation layer is missing: {0}", requiredValidationLayers[i]);
					break;
				}
			}
#endif

			instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(requiredValidationLayers.size());
			instanceCreateInfo.ppEnabledLayerNames = requiredValidationLayers.data();

			// create instance
			Vk_CheckVkResult(typeid(this), vkCreateInstance(&instanceCreateInfo, nullptr, &_instance), "Failed to create instance");

			// create debugger
#ifdef _DEBUG
			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
			debugCreateInfo.messageSeverity =
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
			debugCreateInfo.messageType =
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
			debugCreateInfo.pfnUserCallback = _debugCallback;
			debugCreateInfo.pUserData = this;

			// this thing dynamicaly loads whatever is on the other side of the function pointer
			PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT");
			Vk_CheckVkResult(typeid(this), func != nullptr, "Failed to create debug messanger!");
			func(_instance, &debugCreateInfo, nullptr, &_debugMessenger);
#endif
		}

		std::vector<const char*> _getRequiredExtensions() {
			std::vector<const char*> extensions;
			extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef VK_USE_PLATFORM_WIN32_KHR
			extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
			extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
			extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif
			// add the necessary instance extension for the memory budget thing
			extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
			// check which one of these is the actual one
			// extensions.push_back(VK_NV_EXTERNAL_MEMORY_RDMA_EXTENSION_NAME);
			// extensions.push_back(VK_NV_EXTERNAL_MEMORY_EXTENSION_NAME);

#ifdef _DEBUG
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			extensions.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
			extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif
			return extensions;
		}

#ifdef _DEBUG
		static VKAPI_ATTR VkBool32 VKAPI_CALL _debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageTypes,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData)
		{
			std::string message = UT::GlobalCasters::castValicationLayer(pCallbackData->pMessage);
			switch (messageSeverity) {
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				UT::Ut_Logger::Error(typeid(NoneObj()), message); // red
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				UT::Ut_Logger::Warn(typeid(NoneObj()), message); // yellow
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				UT::Ut_Logger::Log(typeid(NoneObj()), message); // white or other
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				UT::Ut_Logger::Trace(typeid(NoneObj()), message); // white or other
				break;
			default:
				break;
			}

			// always return vk false because the true version is reserved for layer development
			return VK_FALSE;
		}
#endif
	};
}