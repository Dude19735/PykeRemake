#pragma once

#include "../Defines.h"
#include <sstream>
#include <iostream>
#include <queue>
#include <unordered_map>

#include "../Vk_Lib.hpp"
#include "Vk_Instance.hpp"

namespace VK5 {

    enum class Vk_DevicePreference {
        USE_DISCRETE_GPU=0,
        USE_INTEGRATED_GPU=1,
        USE_ANY_GPU=2
    };

    struct OutOfDeviceMemoryException : public std::exception
	{
		const char* what() const throw ()
		{
			return "GPU Device out of memory";
		}
	};

	struct SwapchainSupportDetails {
		VkSurfaceFormatKHR surfaceFormat;
		VkFormat depthFormat;
		VkPresentModeKHR selectedPresentMode;
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
		uint32_t nFramesInFlight;
		bool supportsWideLines;
	};

    class Vk_DeviceLib {
    public:

        struct SingleTimeCommand {
			VkCommandBuffer commandBuffer;
			VkFence fence;
			VkCommandPool commandPool;
            VkQueue queue;
		};

        struct Bridge {
        private:
            std::shared_mutex _mutex;
            std::unique_ptr<std::shared_mutex[]> _frameMutex;
            std::vector<std::queue<std::function<void()>>> _updates;
			std::unique_ptr<bool[]> _rebuildFrame;
			int _currentFrame;
			LWWS::LWWS_Window* _window;
        public:
            Bridge(int nFrames)
			:
			_frameMutex(nullptr),
			_updates({}),
			_rebuildFrame(nullptr), 
			_currentFrame(0), 
			_window(nullptr)
            {
				_rebuildFrame = std::make_unique<bool[]>(nFrames);
                for(uint8_t i=0; i<nFrames; ++i) {
                    auto q = std::queue<std::function<void()>>();
                    _updates.push_back(q);
					_rebuildFrame[i] = false;
                }
                _frameMutex = std::make_unique<std::shared_mutex[]>(nFrames);
            }

            void operator=(const Bridge& old){
                _updates = std::move(old._updates);
                _frameMutex = std::make_unique<std::shared_mutex[]>(_updates.size());
                _currentFrame = old._currentFrame;
            }

            ~Bridge() { 
				_updates.clear(); 
			}

			void assignWindow(LWWS::LWWS_Window* window){
				_window = window;
			}

            void incrFrameNr(){ 
                auto lock = std::lock_guard<std::shared_mutex>(_mutex);
                _currentFrame = (_currentFrame + 1) % _updates.size(); 
            }
            int nFrames() { 
                // this one is constant over the lifetime of this object => no need for mutex
                return _updates.size(); 
            }

            int currentFrame() { 
                auto lock = std::shared_lock<std::shared_mutex>(_mutex);
                return _currentFrame; 
            }
            void setCurrentFrameTo(int index) { 
                auto lock = std::lock_guard<std::shared_mutex>(_mutex);
                _currentFrame = index; 
            }

            void addSameUpdateToAllFrames(const std::function<void()>& func){
                // use frame-local mutex inside addUpdate function
                for(int i=0; i<_updates.size(); ++i){
                    addUpdate(i, func);
                }
            }

            void addUpdateForNextFrame(const std::function<void()>& func){
                // use frame-local mutex inside addUpdate function
                // int nf = (_currentFrame+1) % _updates.size();
                addUpdate(_currentFrame, func);
            }

            void addUpdate(int frameNr, const std::function<void()>& func){
                if(frameNr >= _updates.size()) 
                    UT::Ut_Logger::RuntimeError(typeid(NoneObj), "Tried to add update to non-existing frame Nr {0}. Used number of frames is {1}", frameNr, _updates.size());
                // std::cout << "lock for frame " << frameNr << std::endl;
                auto lock = std::lock_guard<std::shared_mutex>(_frameMutex[frameNr]);
				// std::cout << "locked for frame " << frameNr << std::endl;
                _updates.at(frameNr).push(func);
				// std::cout << "unlock for frame " << frameNr << std::endl;
            }

			void rebuildFrames(){
				if(_window == nullptr){
					UT::Ut_Logger::RuntimeError(typeid(NoneObj), "Bridge has no window. Call device->bridge.assignWindow(_surface->vk_lwws_window())!");
				}

				for(int i=0; i<_updates.size(); ++i){
					auto lock = std::lock_guard<std::shared_mutex>(_frameMutex[i]);	
					_rebuildFrame[i] = true;
				}
				_window->emit_windowEvent_Paint();
			}

			void clearAllQueues(){
				for(int i=0; i<_updates.size(); ++i){
					auto lock = std::lock_guard<std::shared_mutex>(_frameMutex[i]);	
					auto empty = std::queue<std::function<void()>>();
					std::swap(_updates.at(i), empty);
					_rebuildFrame[i] = false;
				}
			}

            bool runCurrentFrameUpdates(){
                auto lock = std::lock_guard<std::shared_mutex>(_frameMutex[_currentFrame]);
				
				if(!_rebuildFrame[_currentFrame]) return false;
				_rebuildFrame[_currentFrame] = false;

                auto& updates = _updates.at(_currentFrame);
				while(!updates.empty()){
					updates.front()();
					updates.pop();
				}

				return true;
            }
        };

        struct QueueFamilyIndex {
			int graphicsFamilyIndex = -1;
			int presentFamilyIndex = -1;
			int transferFamilyIndex = -1;

			bool isComplete() {
				return graphicsFamilyIndex >= 0 && presentFamilyIndex >= 0 && transferFamilyIndex >= 0;
			}
		};

        struct GpuMemoryConfiguration {
			std::uint32_t heapIndex;
			std::string flagsStr;
			std::string flagsStrLong;
		};

		struct GpuMemoryHeapConfiguration {
			std::uint64_t heapSize;
			double heapSizeKb;
			double heapSizeMb;
			double heapSizeGb;
			std::uint64_t heapBudget;
			double heapBudgetKb;
			double heapBudgetMb;
			double heapBudgetGb;
			std::uint64_t heapUsage;
			double heapUsageKb;
			double heapUsageMb;
			double heapUsageGb;
			std::uint32_t heapIndex;
			std::vector<GpuMemoryConfiguration*> heapConfigs;
		};

        /**
		* Some abstraction for physical devices
		* Doesn't deserve an own class though
		*/
		struct PhysicalDevice {
			int physicalDeviceIndex;
			VkPhysicalDevice physicalDevice;
			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures deviceFeatures;
			VkSampleCountFlagBits maxUsableSampleCount;
			QueueFamilyIndex queueFamilyIndices;
			std::vector<VkQueueFamilyProperties> queueFamilyProperties;
			std::vector<VkExtensionProperties> availableExtensions;

			bool supportsGraphicsQueue() {
				return queueFamilyIndices.graphicsFamilyIndex >= 0;
			}

			bool supportsPresentationQueue() {
				return queueFamilyIndices.presentFamilyIndex >= 0;
			}

			bool supportsSwapchainExtension() {
				for (const auto& extension : availableExtensions) {
					if (strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
						return true;
					}
				}
				return false;
			}

			bool supportsMemoryBudgetExtension() {
				for (const auto& extension : availableExtensions) {
					if (strcmp(extension.extensionName, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME) == 0) {
						return true;
					}
				}
				return false;
			}

			bool supportsWideLines() {
				return deviceFeatures.wideLines == VK_TRUE;
			}

			std::vector<const char*> getMinimumRequiredExtensions() {
				return { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME };
			}
		};

        typedef std::unordered_map<PhysicalDevice*, std::map<VkMemoryPropertyFlags, GpuMemoryConfiguration>> TGpuMemoryConfig;
    	typedef std::unordered_map<PhysicalDevice*, std::vector<GpuMemoryHeapConfiguration>> TGpuHeapConfig;

		static void setGpuMemoryConfig(std::vector<PhysicalDevice>& physicalDevices, TGpuHeapConfig& gpuHeapConfig, TGpuMemoryConfig& gpuMemoryConfig) {
			for (auto& device : physicalDevices) {
				VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
				vkGetPhysicalDeviceMemoryProperties(device.physicalDevice, &deviceMemoryProperties);

				for (uint32_t i = 0; i < deviceMemoryProperties.memoryHeapCount; ++i) {
					double s = static_cast<double>(deviceMemoryProperties.memoryHeaps[i].size);
					double s_kb = Vk_Lib::round(s / 1.0e3, 2);
					double s_mb = Vk_Lib::round(s / 1.0e6, 2);
					double s_gb = Vk_Lib::round(s / 1.0e9, 2);
					gpuHeapConfig[&device].push_back(GpuMemoryHeapConfiguration{
						.heapSize = deviceMemoryProperties.memoryHeaps[i].size,
						.heapSizeKb = s_kb,
						.heapSizeMb = s_mb,
						.heapSizeGb = s_gb,
						.heapIndex = static_cast<uint32_t>(i),
						.heapConfigs = std::vector<GpuMemoryConfiguration*>()
						});
				}

				for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; ++i) {
					auto h = deviceMemoryProperties.memoryTypes[i];
					std::string propStrShort;
					std::string propStrLong;
					devicePropertyFlagToString(h.propertyFlags, propStrShort, propStrLong);

					gpuMemoryConfig[&device].insert({
						h.propertyFlags,
						GpuMemoryConfiguration{
							.heapIndex = h.heapIndex,
							.flagsStr = propStrShort,
							.flagsStrLong = propStrLong
						}
						});

					gpuHeapConfig[&device].at(h.heapIndex).heapConfigs.push_back(&gpuMemoryConfig[&device].at(h.propertyFlags));
				}
			}
		}

        static void updateGpuHeapUsageStats(std::vector<PhysicalDevice>& physicalDevices, TGpuHeapConfig& gpuHeapConfig) {

			VkPhysicalDeviceMemoryProperties2 props;
			props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;

			VkPhysicalDeviceMemoryBudgetPropertiesEXT next;
			next.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT;
			next.pNext = VK_NULL_HANDLE;
			props.pNext = &next;

			for(auto& device : physicalDevices) {
				vkGetPhysicalDeviceMemoryProperties2(device.physicalDevice, &props);

				for (uint32_t i = 0; i < props.memoryProperties.memoryHeapCount; ++i) {
					double budget = static_cast<double>(next.heapBudget[i]);
					double budget_kb = Vk_Lib::round(budget / 1.0e3, 2);
					double budget_mb = Vk_Lib::round(budget / 1.0e6, 2);
					double budget_gb = Vk_Lib::round(budget / 1.0e9, 2);
					gpuHeapConfig[&device][i].heapBudget = next.heapBudget[i];
					gpuHeapConfig[&device][i].heapBudgetKb = budget_kb;
					gpuHeapConfig[&device][i].heapBudgetMb = budget_mb;
					gpuHeapConfig[&device][i].heapBudgetGb = budget_gb;

					double usage = static_cast<double>(next.heapUsage[i]);
					double usage_kb = Vk_Lib::round(usage / 1.0e3, 2);
					double usage_mb = Vk_Lib::round(usage / 1.0e6, 2);
					double usage_gb = Vk_Lib::round(usage / 1.0e9, 2);
					gpuHeapConfig[&device][i].heapUsage = next.heapUsage[i];
					gpuHeapConfig[&device][i].heapUsageKb = usage_kb;
					gpuHeapConfig[&device][i].heapUsageMb = usage_mb;
					gpuHeapConfig[&device][i].heapUsageGb = usage_gb;
				}
			}
		}

        static void printActiveDeviceMemoryProperties(std::vector<PhysicalDevice>& physicalDevices, TGpuHeapConfig& gpuHeapConfig, std::ostream& stream = std::cout) {
			updateGpuHeapUsageStats(physicalDevices, gpuHeapConfig);
			tabulate::Table table;
			table.add_row({"GPU", "Heap Size", "Usage", "Heap Config Possibilities"});
			table.format().font_style({ tabulate::FontStyle::bold })
				.border_top(" ").border_bottom(" ")
				.border_left(" ").border_right(" ")
				.corner(" ");
			table[0].format()
				.padding_top(1)
				.padding_bottom(1)
				.font_align(tabulate::FontAlign::center)
				.font_style({ tabulate::FontStyle::underline })
				.font_background_color(tabulate::Color::red);

			for (auto& item : gpuHeapConfig) {
				for (size_t i = 0; i < item.second.size(); ++i) {
					auto h = item.second.at(i); // _gpuHeapConfig[_activePhysicalDevice].at(i);
					std::string s_kb = Vk_Lib::rightCrop(h.heapSizeKb) + "[Kb]";
					std::string s_mb = Vk_Lib::rightCrop(h.heapSizeMb) + "[MB]";
					std::string s_gb = Vk_Lib::rightCrop(h.heapSizeGb) + "[GB]";

					std::string usage_kb = Vk_Lib::rightCrop(h.heapUsageKb) + " / " + Vk_Lib::rightCrop(h.heapBudgetKb) + "[Kb]";
					std::string usage_mb = Vk_Lib::rightCrop(h.heapUsageMb) + " / " + Vk_Lib::rightCrop(h.heapBudgetMb) + "[MB]";
					std::string usage_gb = Vk_Lib::rightCrop(h.heapUsageGb) + " / " + Vk_Lib::rightCrop(h.heapBudgetGb) + "[GB]";

					std::string props = "";
					for (auto conf : h.heapConfigs) {
						props.append(conf->flagsStr);
						props.append("\n");
					}
					//std::cout << std::string(item.first->deviceProperties.deviceName) << std::endl;
					table.add_row({
						std::string(item.first->deviceProperties.deviceName),
						s_kb + "\n" + s_mb + "\n" + s_gb,
						usage_kb + "\n" + usage_mb + "\n" + usage_gb,
						props });
				}
			}

			table.column(0).format().font_color(tabulate::Color::red);
			table.column(1).format().font_color(tabulate::Color::blue);
			table.column(2).format().font_color(tabulate::Color::cyan);
			table.column(3).format().font_color(tabulate::Color::yellow);

			table[0][0].format().font_background_color(tabulate::Color::red).font_color(tabulate::Color::white);
			table[0][1].format().font_background_color(tabulate::Color::blue).font_color(tabulate::Color::white);
			table[0][2].format().font_background_color(tabulate::Color::cyan).font_color(tabulate::Color::blue);
			table[0][3].format().font_background_color(tabulate::Color::yellow).font_color(tabulate::Color::blue);

			stream << table << std::endl;
		}

        static VkBool32 getSupportedDepthFormat(VkFormat& depthFormat, Vk_DeviceLib::PhysicalDevice* pDev)
		{
			// Since all depth formats may be optional, we need to find a suitable depth format to use
			// Start with the highest precision packed format
			std::vector<VkFormat> depthFormats = {
				VK_FORMAT_D32_SFLOAT_S8_UINT,
				VK_FORMAT_D32_SFLOAT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D16_UNORM_S8_UINT,
				VK_FORMAT_D16_UNORM
			};

			for (auto& format : depthFormats)
			{
				VkFormatProperties formatProps;
				vkGetPhysicalDeviceFormatProperties(pDev->physicalDevice, format, &formatProps);
				// Format must support depth stencil attachment for optimal tiling
				if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
				{
					depthFormat = format;
					return true;
				}
			}

			return false;
		}

        static SwapchainSupportDetails swapchainSupport(PhysicalDevice* pDev, VkSurfaceKHR surface, bool multiImageBuffering, Bridge& bridge) {
            SwapchainSupportDetails swapchainSupportDetails;
			if(surface == nullptr){
				UT::Ut_Logger::RuntimeError(typeid(NoneObj), "No surface passed to vk_swapchainSupport query!");
			}

			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
				pDev->physicalDevice, surface, &swapchainSupportDetails.capabilities
			);

			// query supported surface formats
			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(
				pDev->physicalDevice, surface, &formatCount, nullptr
			);

			// list all supported surface formats
			if (formatCount != 0) {
				swapchainSupportDetails.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(
					pDev->physicalDevice, surface, &formatCount, swapchainSupportDetails.formats.data()
				);
			}

			// query all available surface presentation modes
			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(
				pDev->physicalDevice, surface, &presentModeCount, nullptr
			);

			// list all supported surface presentation modes
			if (presentModeCount != 0) {
				swapchainSupportDetails.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(
					pDev->physicalDevice, surface, &presentModeCount, swapchainSupportDetails.presentModes.data()
				);
			}

			// get the capabilities for the size of the viewport
			// =================================================
			//auto& cap = swapchainSupportDetails.capabilities;
			//auto wh = surface->vk_surfaceExtent();

			////swapchainSupportDetails
			//if (cap.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			//	// if the maximal extend is not the maximal value of an uint32 then just take the current size in here
			//	swapchainSupportDetails.extent2d = cap.currentExtent;
			//}
			//else {
			//	// otherwise make it as large as can possibly fit on the screen
			//	// get the size of the window from glfw

			//	VkExtent2D actualExtent = {
			//		static_cast<uint32_t>(wh[0]),
			//		static_cast<uint32_t>(wh[1])
			//	};

			//	actualExtent.width = std::max(cap.minImageExtent.width, std::min(cap.maxImageExtent.width, actualExtent.width));
			//	actualExtent.height = std::max(cap.minImageExtent.height, std::min(cap.maxImageExtent.height, actualExtent.height));
			//	swapchainSupportDetails.extent2d = actualExtent;
			//}

			// get the possible surface format
			// ===============================
			// this is the best possibility
			bool formatChosen = false;
			auto& forms = swapchainSupportDetails.formats;
			if (forms.size() == 1 && forms[0].format == VK_FORMAT_UNDEFINED) {
				swapchainSupportDetails.surfaceFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
				formatChosen = true;
			}
			else {
				// if there are multiple formats available, check if the preferred format/colorspace combination is available
				for (const auto& availableFormat : forms) {
					if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
						swapchainSupportDetails.surfaceFormat = availableFormat;
						formatChosen = true;
						break;
					}
				}
			}
			if(!formatChosen) {
				// otherwise just return whatever is found
				swapchainSupportDetails.surfaceFormat = forms[0];
			}

			// get best supported depth format
			// ================================
			if (!getSupportedDepthFormat(swapchainSupportDetails.depthFormat, pDev)) {
				UT::Ut_Logger::RuntimeError(typeid(NoneObj), UT::GlobalCasters::castHighlightRed("No supported depth formats found!"));
			}

			// choose the possible present mode
			// ================================
			bool presentModeChosen = false;
			auto& pModes = swapchainSupportDetails.presentModes;
			for (const auto& availablePresentMode : pModes) {
				// VK_PRESENT_MODE_MAILBOX_KHR is the best one, use it if available
				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
					swapchainSupportDetails.selectedPresentMode = availablePresentMode;
					presentModeChosen = true;
					break;
				}

				// some drivers don't properly support fifo, so prefere this one if present
				// with no mailbox
				else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
					swapchainSupportDetails.selectedPresentMode = availablePresentMode;
					presentModeChosen = true;
					break;
				}
			}
			if (!presentModeChosen) {
				swapchainSupportDetails.selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR;
			}

			// register if the current device supports wide lines
			if (pDev->supportsWideLines()) {
				swapchainSupportDetails.supportsWideLines = true;
			}

			// if we require double buffering, check if that is possible
			// =========================================================
			// set the nFramesInFlight as either the N_FRAMES_IN_FLIGHT or whatever the hardware supports
			if (multiImageBuffering) {
				// for all intents and purposes, this should be two
				//swapchainSupportDetails.nFramesInFlight = swapchainSupportDetails.capabilities.minImageCount + 1;

				// check if imageCount = 2 is too much... or else, keep imageCount = 2
				const auto& caps = swapchainSupportDetails.capabilities;
				swapchainSupportDetails.nFramesInFlight = std::max<uint32_t>(2, caps.minImageCount);
				if (caps.maxImageCount > 0 && swapchainSupportDetails.nFramesInFlight > caps.maxImageCount) {
					UT::Ut_Logger::RuntimeError(typeid(NoneObj), "Double Buffering not supported by GPU. Max supported Frames-In_Flight are " + caps.maxImageCount);
					//swapchainSupportDetails.nFramesInFlight = caps.maxImageCount;
				}
			}
			else {
				const auto& caps = swapchainSupportDetails.capabilities;
				swapchainSupportDetails.nFramesInFlight = std::max<uint32_t>(1, caps.minImageCount);
				if(caps.minImageCount > 1){
					UT::Ut_Logger::Warn(typeid(NoneObj), "Single image not supported by swapchain. Minimum " + std::to_string(caps.minImageCount) + " images required! Setting nFramesInFlight to "  + std::to_string(caps.minImageCount) + ".");
				}
			}

            bridge = Bridge(swapchainSupportDetails.nFramesInFlight);

			return swapchainSupportDetails;
			// }
		}

        static bool findGpuForDevicePreferences(
            const Vk_DeviceLib::PhysicalDevice& physicalDevice,
            Vk_DevicePreference devicePreference
        ){
			if(devicePreference == Vk_DevicePreference::USE_ANY_GPU){
				return (
					(physicalDevice.deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) || 
					(physicalDevice.deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
				);
			}

			if(devicePreference == Vk_DevicePreference::USE_DISCRETE_GPU){
				return (
					(physicalDevice.deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				);
			}

			if(devicePreference == Vk_DevicePreference::USE_INTEGRATED_GPU){
				return ( 
					(physicalDevice.deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
				);
			}

			return false;
		}

        static bool setPhysicalDevice(
            Vk_Instance* instance, 
            VkSurfaceKHR surface,
            std::vector<PhysicalDevice>& physicalDevices,
            bool multiImageBuffering,
            Vk_DevicePreference devicePreference,
            /*inout*/ Bridge& bridge,
            /*out*/ int& physicalDeviceIndex,
			/*out*/ PhysicalDevice*& activePhysicalDevice
        ){
			uint32_t deviceCount = 0;
			Vk_CheckVkResult(typeid(NoneObj), 
				vkEnumeratePhysicalDevices(instance->vk_instance(), &deviceCount, nullptr),
				"Failed to enumerate physical devices"
			);

			if (deviceCount == 0)
				UT::Ut_Logger::RuntimeError(typeid(NoneObj), UT::GlobalCasters::castHighlightRed("Failed to find devices with Vulkan support!"));

			std::vector<VkPhysicalDevice> vkPhysicalDevices(deviceCount);
			VkResult res = vkEnumeratePhysicalDevices(instance->vk_instance(), &deviceCount, vkPhysicalDevices.data());
			if(res == VK_INCOMPLETE){
				std::string msg = "Fewer GPUs than actually presend in hardware were detected by the graphics driver. Note that in the newest version of Windows, the GPU is selected per App and all GPUs are visible by default. It can be that the vendor of one of the GPUs hides the respective other GPU. This causes VK_INCOMPLETE to be returned. If you use a dual GPU system, select the GPU that does not cause these problems and select the GPU for the viewer using Vk_DevicePreference.";
				UT::Ut_Logger::RuntimeError(typeid(NoneObj), msg);
			}
			else{
				Vk_CheckVkResult(typeid(NoneObj), res, "Failed to load physical devices");
			}

			// iterate over all available physical devices to find a good one
			int i = 0;
			for (const auto& physicalDevice : vkPhysicalDevices) {
				physicalDevices.push_back(PhysicalDevice());

				// assign current physical device
				physicalDevices[i].physicalDevice = physicalDevice;
				physicalDevices[i].physicalDeviceIndex = i;

				// query basic physical device stuff like type and supported vulkan version
				vkGetPhysicalDeviceProperties(
					physicalDevices[i].physicalDevice,
					&physicalDevices[i].deviceProperties
				);

				// query exotic stuff about the device like viewport rendering or 64 bit floats
				vkGetPhysicalDeviceFeatures(
					physicalDevices[i].physicalDevice,
					&physicalDevices[i].deviceFeatures
				);

				// get the maximum usable sample count
				VkSampleCountFlags counts = std::min(
					physicalDevices[i].deviceProperties.limits.framebufferColorSampleCounts,
					physicalDevices[i].deviceProperties.limits.framebufferDepthSampleCounts
				);

				if (counts & VK_SAMPLE_COUNT_64_BIT) { physicalDevices[i].maxUsableSampleCount = VK_SAMPLE_COUNT_64_BIT; }
				else if (counts & VK_SAMPLE_COUNT_32_BIT) { physicalDevices[i].maxUsableSampleCount = VK_SAMPLE_COUNT_32_BIT; }
				else if (counts & VK_SAMPLE_COUNT_16_BIT) { physicalDevices[i].maxUsableSampleCount = VK_SAMPLE_COUNT_16_BIT; }
				else if (counts & VK_SAMPLE_COUNT_8_BIT) { physicalDevices[i].maxUsableSampleCount = VK_SAMPLE_COUNT_8_BIT; }
				else if (counts & VK_SAMPLE_COUNT_4_BIT) { physicalDevices[i].maxUsableSampleCount = VK_SAMPLE_COUNT_4_BIT; }
				else if (counts & VK_SAMPLE_COUNT_2_BIT) { physicalDevices[i].maxUsableSampleCount = VK_SAMPLE_COUNT_2_BIT; }
				else physicalDevices[i].maxUsableSampleCount = VK_SAMPLE_COUNT_1_BIT;

				// query device queue indices for graphics and present family
				uint32_t queueFamilyCount = 0;
				vkGetPhysicalDeviceQueueFamilyProperties(
					physicalDevices[i].physicalDevice,
					&queueFamilyCount,
					nullptr
				);

				physicalDevices[i].queueFamilyProperties.resize(queueFamilyCount);
				vkGetPhysicalDeviceQueueFamilyProperties(
					physicalDevices[i].physicalDevice,
					&queueFamilyCount,
					physicalDevices[i].queueFamilyProperties.data()
				);

				// check for graphics and presentation support
				VkBool32 presentSupport = false;
				int index = 0;
				for (const auto& queueFamily : physicalDevices[i].queueFamilyProperties) {
					// check if given device supports graphics
					if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT && physicalDevices[i].queueFamilyIndices.graphicsFamilyIndex < 0) {
						physicalDevices[i].queueFamilyIndices.graphicsFamilyIndex = index;
					}

					// check if given device supports presentation to a particular given surface
					vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[i].physicalDevice, index, surface, &presentSupport);
					if (queueFamily.queueCount > 0 && presentSupport && physicalDevices[i].queueFamilyIndices.presentFamilyIndex < 0) {
						physicalDevices[i].queueFamilyIndices.presentFamilyIndex = index;
					}

					if(queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT){
						physicalDevices[i].queueFamilyIndices.transferFamilyIndex = index;
					}
					index++;
				}

				// if both queues have an index that is greater than 0, break
				if (!physicalDevices[i].queueFamilyIndices.isComplete()) {
					UT::Ut_Logger::RuntimeError(typeid(NoneObj), "No suitable graphics device found");
				}

				// query all available extensions
				uint32_t extensionCount;
				vkEnumerateDeviceExtensionProperties(
					physicalDevices[i].physicalDevice,
					nullptr,
					&extensionCount,
					nullptr
				);

				physicalDevices[i].availableExtensions.resize(extensionCount);
				vkEnumerateDeviceExtensionProperties(
					physicalDevices[i].physicalDevice,
					nullptr,
					&extensionCount,
					physicalDevices[i].availableExtensions.data()
				);

				i++;
			}

			// check to see if there is one physical device suitable for graphics application
			std::vector<PhysicalDevice>::iterator pDev = physicalDevices.begin();
			int index = 0;

			// prefer dedicated GPU vs integrated GPU
			bool found = false;
			while (pDev != physicalDevices.end()) {
				const SwapchainSupportDetails& support = swapchainSupport(&(*pDev), surface, multiImageBuffering, bridge);
				found = pDev->supportsGraphicsQueue()
					&& pDev->supportsPresentationQueue()
					&& pDev->supportsSwapchainExtension()
					&& pDev->supportsMemoryBudgetExtension()
					&& !support.formats.empty()
					&& !support.presentModes.empty()
					&& pDev->deviceFeatures.samplerAnisotropy
					&& findGpuForDevicePreferences(*pDev, devicePreference)
					&& pDev->deviceFeatures.geometryShader;

				if (found) {
					physicalDeviceIndex = index;
					activePhysicalDevice = &physicalDevices.at(index);
					return true;
				}

				index++;
				pDev++;
			}

			return false;
		}

        static void submitWork(
            VkDevice device,
            VkCommandBuffer cmdBuffer,
            VkQueue queue
        ) {
			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmdBuffer;

			VkFenceCreateInfo fenceCreateInfo{};
			fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceCreateInfo.flags = 0;
			VkFence fence;

			Vk_CheckVkResult(typeid(NoneObj), vkCreateFence(device, &fenceCreateInfo, nullptr, &fence), "Unable to create fence");

			VkResult res = Vk_ThreadSafe::Vk_ThreadSafe_QueueSubmit(queue, 1, &submitInfo, fence);
			Vk_CheckVkResult(typeid(NoneObj), res, "Unable to submit fence to queue");

			Vk_CheckVkResult(typeid(NoneObj), vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX), "Unable to wait for fences");
			vkDestroyFence(device, fence, nullptr);
		}

        static void copyDeviceBufferToVector(
            VkDevice device,
            void* dstPtr, 
            VkDeviceMemory deviceBufferMemory, 
            VkDeviceSize size
        ) {
			const char* data;
			// Map image memory so we can start copying from it
			vkMapMemory(device, deviceBufferMemory, 0, size, 0, (void**)&data);

			memcpy(dstPtr, (void*)data, (size_t)size);

			// Clean up resources
			vkUnmapMemory(device, deviceBufferMemory);
			// Vk_ThreadSafe::Vk_ThreadSafe_QueueWaitIdle(_graphicsQueues[0]);
		}

        static void insertImageMemoryBarrier(
			VkCommandBuffer cmdbuffer,
			VkImage image,
			VkAccessFlags srcAccessMask,
			VkAccessFlags dstAccessMask,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask,
			VkImageSubresourceRange subresourceRange)
		{
			VkImageMemoryBarrier imageMemoryBarrier{};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.srcAccessMask = srcAccessMask;
			imageMemoryBarrier.dstAccessMask = dstAccessMask;
			imageMemoryBarrier.oldLayout = oldImageLayout;
			imageMemoryBarrier.newLayout = newImageLayout;
			imageMemoryBarrier.image = image;
			imageMemoryBarrier.subresourceRange = subresourceRange;

			vkCmdPipelineBarrier(
				cmdbuffer,
				srcStageMask,
				dstStageMask,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier
			);
		}

		static void createCopyToBufferCommand(
			VkCommandBuffer& dstComdBuffer,
			VkImage& srcImage,
			VkBuffer& dstBuffer,
			VkImageAspectFlags aspectFlags,
			VkExtent3D imageExtent
		) {
			VkBufferImageCopy copyRegion = {};
			copyRegion.bufferOffset = 0;
			copyRegion.bufferImageHeight = 0;
			copyRegion.bufferRowLength = 0;
			copyRegion.imageSubresource.aspectMask = aspectFlags;
			copyRegion.imageSubresource.mipLevel = 0;
			copyRegion.imageSubresource.baseArrayLayer = 0;
			copyRegion.imageSubresource.layerCount = 1;
			copyRegion.imageOffset = VkOffset3D{ 0, 0, 0 };

			copyRegion.imageExtent = imageExtent; 

			vkCmdCopyImageToBuffer(
				dstComdBuffer,
				srcImage,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				dstBuffer,
				1,
				&copyRegion
			);
		}

        static void createCopyImgToBufferCommand(
            VkDevice device,
            VkCommandPool commandPool,
			VkCommandBuffer& copyCmdBuffer,
			VkImage srcImage,
			VkBuffer dstBuffer,
			VkExtent3D imageExtent
		) {
			// Do the actual blit from the offscreen image to our host visible destination image
			VkCommandBufferAllocateInfo cmdBufAllocateInfo{};
			cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdBufAllocateInfo.commandPool = commandPool;
			cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmdBufAllocateInfo.commandBufferCount = 1;

			Vk_CheckVkResult(typeid(NoneObj), vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &copyCmdBuffer), "Unable to command buffers");

			VkCommandBufferBeginInfo cmdBufferBeginInfo{};
			cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			Vk_CheckVkResult(typeid(NoneObj), vkBeginCommandBuffer(copyCmdBuffer, &cmdBufferBeginInfo), "Unable to begin command buffer recording");

			insertImageMemoryBarrier(
				copyCmdBuffer,
				srcImage,
				0,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
			);

			createCopyToBufferCommand(
				copyCmdBuffer,
				srcImage,
				dstBuffer,
				VK_IMAGE_ASPECT_COLOR_BIT,
				imageExtent
			);

			insertImageMemoryBarrier(
				copyCmdBuffer,
				srcImage,
				0,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
			);

			Vk_CheckVkResult(typeid(NoneObj), vkEndCommandBuffer(copyCmdBuffer), "Unable to end command buffer recording");// Transition destination image to transfer destination layout
		}

        static VkImageView createImageView(
            VkDevice device,
			VkImage image,
			VkFormat format,
			VkImageAspectFlags aspectFlags,
			uint32_t miplevels
		) {
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = image;
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = format;

			// use default mappings for every color channel
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createInfo.subresourceRange.aspectMask = aspectFlags;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = miplevels;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			VkImageView imageView;
			Vk_CheckVkResult(typeid(NoneObj), vkCreateImageView(device, &createInfo, nullptr, &imageView), "Failed to create image view");

			return imageView;
		}

        static uint32_t findMemoryType(
            const PhysicalDevice* activePhysicalDevice,
			uint32_t typeFilter,
			VkMemoryPropertyFlags properties
		) {
			// two different types of entries:
			// memoryTypes: different types of memory within heaps => array of VkMemoryType
			// memoryHeaps: distinct memory resources like dedicated VRAM and swap in RAM where a heap is located may affect performance
			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(activePhysicalDevice->physicalDevice, &memProperties);

			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
				// bitmask check the memory type
				if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
					return i;
				}
			}

			UT::Ut_Logger::RuntimeError(typeid(NoneObj), "Failed to find suitable memory type!");

			return 0;
		}

        static void createImage(
            const PhysicalDevice* activePhysicalDevcie,
            VkDevice device,
			uint32_t width,
			uint32_t height,
			uint32_t mipLevels,
			VkSampleCountFlagBits numSamples,
			VkFormat format,
			VkImageTiling tiling,
			VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkImage& image,
			VkDeviceMemory& imageMemory
		) {
			// set up actual image object
			VkImageCreateInfo imageInfo = {};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = static_cast<uint32_t>(width);
			imageInfo.extent.height = static_cast<uint32_t>(height);
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = mipLevels;
			imageInfo.arrayLayers = 1;
			imageInfo.format = format;
			imageInfo.tiling = tiling;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = usage;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.samples = numSamples;
			imageInfo.flags = 0;

			Vk_CheckVkResult(typeid(NoneObj), vkCreateImage(device, &imageInfo, nullptr, &image), "Failed to create image");

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(device, image, &memRequirements);

			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = findMemoryType(activePhysicalDevcie, memRequirements.memoryTypeBits, properties);

			if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
				UT::Ut_Logger::RuntimeError(typeid(NoneObj), "failed to allocate image memory!");
			}

			// bind image to staging buffer memory
			vkBindImageMemory(device, image, imageMemory, 0);
		}

        static SingleTimeCommand beginSingleTimeCommands(
            VkDevice device,
            VkCommandPool commandPool,
            VkQueue queue
        ) {

			// // memory transfer operations are executed using command buffers
			// // => allocate temporary command buffer
			VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = commandPool; // may want to specify another command pool for this kind of short lived command buffers
			allocInfo.commandBufferCount = 1;
			VkCommandBuffer buffer;
			Vk_CheckVkResult(typeid(NoneObj), vkAllocateCommandBuffers(device, &allocInfo, &buffer), "Unable to allocate single-time-command-buffer");

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			// we must create the fences in the signaled state to ensure that on the first call to vkWaitForFences won't wait indefinitely
			// fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT
			VkFence fence;
			Vk_CheckVkResult(typeid(NoneObj), vkCreateFence(device, &fenceInfo, nullptr, &fence),"Failed to create synchronization objects for a frame!");

			SingleTimeCommand singleTimeCommand = {
				.commandBuffer=buffer, 
				.fence=fence,
				.commandPool=commandPool,
                .queue=queue
			};
			//_singleTimeCommandBuffer.push_back(singleTimeCommand);

			// memory transfer operations are executed using command buffers
			// => allocate temporary command buffer
			// VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
			// allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			// allocInfo.commandPool = selectCommandPool(command); // may want to specify another command pool for this kind of short lived command buffers
			// allocInfo.commandBufferCount = 1;

			// VkCommandBuffer commandBuffer;
			// vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);

			// start recoding the command
			// VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT not necessary, because this is an execute and wait until finished kind
			// of methode, therefore use
			// VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT to tell the driver about our intentions
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			// VkResult status = vkGetFenceStatus(_device, _singleTimeFences[_singleTimeIndex]);

			// std::cout << "##########################" << std::endl << " Wait for fence " << _singleTimeIndex << " (current status: " << static_cast<int64_t>(status) << ")" << std::endl;
			// VkResult res = vkWaitForFences(_device, 1, &_singleTimeFences[_singleTimeIndex], VK_TRUE, UINT64_MAX);
			// status = vkGetFenceStatus(_device, _singleTimeFences[_singleTimeIndex]);
			// std::cout << "##########################" << std::endl << " got fence " << _singleTimeIndex << " (current status: " << static_cast<int64_t>(status) << ")" << std::endl;
			// if(res == VK_TIMEOUT){
			// 	UT::Ut_Logger::RuntimeError(typeid(this), "Single time command timeout for index [{0}]", _singleTimeIndex);
			// }
			// else if (res != VK_SUCCESS) {
			// 	UT::Ut_Logger::RuntimeError(typeid(this), "Waiting for single time fence {0} had catastrphic result ({1})!", _singleTimeIndex, static_cast<int64_t>(res));
			// }

			// std::cout << "#############@@@##########" << std::endl << " Submit to fence " << _singleTimeIndex << std::endl;
			// vkResetFences(_device, 1, &_singleTimeFences[_singleTimeIndex]);
			// std::cout << "#############@@@##########" << std::endl << " reset fence " << _singleTimeIndex << std::endl;

			vkBeginCommandBuffer(singleTimeCommand.commandBuffer, &beginInfo); // begin recording command

			// singleTimeIndex = _singleTimeIndex;
			// _singleTimeIndex++;
			// _singleTimeIndex %= _singleTimeBufferCount;
			// potentially select another command buffer
			return singleTimeCommand;
		}

		static void endSingleTimeCommands(
            VkDevice device,
            const SingleTimeCommand& singleTimeCommand
        ) {
			vkEndCommandBuffer(singleTimeCommand.commandBuffer); // end recording command

			// submit the command to the graphics queue
			VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &singleTimeCommand.commandBuffer;

			auto t1 = std::chrono::high_resolution_clock::now();
			// std::cout << "hello world" << std::endl;
			VkResult res = Vk_ThreadSafe::Vk_ThreadSafe_QueueSubmit(singleTimeCommand.queue, 1, &submitInfo, nullptr);
			if(res != VK_SUCCESS){
				if(res == VK_ERROR_OUT_OF_HOST_MEMORY){
					UT::Ut_Logger::RuntimeError(typeid(NoneObj), "vkQueueSubmit failed with VK_ERROR_OUT_OF_HOST_MEMORY ({0})!", static_cast<int64_t>(res));
				}
				else if(res == VK_ERROR_OUT_OF_DEVICE_MEMORY){
					UT::Ut_Logger::RuntimeError(typeid(NoneObj), "vkQueueSubmit failed with VK_ERROR_OUT_OF_DEVICE_MEMORY ({0})!", static_cast<int64_t>(res));
				}
				else{
					UT::Ut_Logger::RuntimeError(typeid(NoneObj), "vkQueueSubmit failed with {0}!", static_cast<int64_t>(res));
				}
			}
			// Vk_ThreadSafe::Vk_ThreadSafe_QueueWaitIdle(_transferQueues[0]);
			// auto t2 = std::chrono::high_resolution_clock::now();
			// std::cout << "#############@@@##########" << std::endl << " command finished - " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << std::endl;
			Vk_ThreadSafe::Vk_ThreadSafe_QueueWaitIdle(singleTimeCommand.queue);

			// res = vkWaitForFences(_device, 1, &singleTimeCommand.fence, VK_TRUE, UINT64_MAX);
			// if(res == VK_TIMEOUT){
			// 	UT::Ut_Logger::RuntimeError(typeid(this), "Single time command timeout for index");
			// }
			// else if (res != VK_SUCCESS) {
			// 	UT::Ut_Logger::RuntimeError(typeid(this), "Waiting for single time fence had catastrphic result ({0})!", static_cast<int64_t>(res));
			// }
			// auto lock = std::lock_guard<std::mutex>(queue_submit_mutex);
			// vk_cleanSingleTimeCommands(true);
			// free command buffer memory
			vkFreeCommandBuffers(device, singleTimeCommand.commandPool, 1, &singleTimeCommand.commandBuffer);
			vkDestroyFence(device, singleTimeCommand.fence, nullptr);
		}

        static void transitionImageLayout(
			// CommandCapabilities command,
            VkDevice device,
            VkCommandPool commandPool,
            VkQueue queue,
			VkImage image,
			VkFormat format,
			VkImageLayout oldLayout,
			VkImageLayout newLayout,
			uint32_t mipLevels
		) {
			// record command to move the image from buffer memory to real image memory where the shader
			// can access it using a sampler
			SingleTimeCommand singleTimeCommand = beginSingleTimeCommands(device, commandPool, queue);

			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // must use this if there is no transition between two differen queue families
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // must use this if there is no transition between two differen queue families
			barrier.image = image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // something funny, maybe find out what it's supposed to be
			barrier.subresourceRange.baseMipLevel = 0; // mo mipmapping so only one basic layer with index 0
			barrier.subresourceRange.levelCount = 1; // one level with texel colors
			barrier.subresourceRange.baseArrayLayer = 0; // image is not formatted as an array
			barrier.subresourceRange.layerCount = mipLevels; // one layer of texels

			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;

			// initiate depth bufferig layout, newLayout can be used as initial layout because the initial contents of the depth image matter
			if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

				// check if has stencil component
				bool hasStencilComponent =
					(format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT);

				if (hasStencilComponent) {
					barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
				}
			}
			else {
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			}

			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				barrier.srcAccessMask = 0; // no need to wait for anything
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // wait until write operation is complete
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // reading happens here
				// writing happens in VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			else {
				UT::Ut_Logger::RuntimeError(typeid(NoneObj), "Unsupported layout transition!");
			}

			vkCmdPipelineBarrier(
				singleTimeCommand.commandBuffer,
				sourceStage, destinationStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			endSingleTimeCommands(device, singleTimeCommand);
		}

        static void createSwapchainResource(
            const VkDevice device,
            const PhysicalDevice* activePhysicalDevice,
			const VkImageTiling tiling,
			const VkImageUsageFlags imageUsageFlags,
			const VkMemoryPropertyFlags usageFlags,
			const VkImageAspectFlags imageAspectFlags,
			const VkExtent2D& extent2D,
			const VkFormat& colorFormat,
			const uint32_t miplevels,
			const VkImageLayout oldLayout,
			const VkImageLayout newLayout,
            VkCommandPool commandPool,
            VkQueue queue,
			VkImage& swapchainImage,
			VkDeviceMemory& swapchainImageMemory,
			VkImageView& swapchainImageView
		) {
			//!create image
			createImage(
                activePhysicalDevice,
                device,
				extent2D.width,
				extent2D.height,
				miplevels,
				activePhysicalDevice->maxUsableSampleCount,
				colorFormat,
				tiling,
				imageUsageFlags,
				usageFlags,
				swapchainImage,
				swapchainImageMemory
			);

			//! create image view
			swapchainImageView = createImageView(
                device,
				swapchainImage,
				colorFormat,
				imageAspectFlags,
				miplevels
			);

			//! transition image view to some faster memory type
			transitionImageLayout(
                device,
                commandPool,
                queue,
				swapchainImage,
				colorFormat,
				oldLayout,
				newLayout,
				miplevels
			);
		}

		static void copyBuffer(
			VkDevice device,
            VkCommandPool commandPool,
            VkQueue queue,
			VkBuffer& srcBuffer,
			VkDeviceSize srcOffset,
			VkBuffer& dstBuffer,
			VkDeviceSize dstOffset,
			VkDeviceSize size,
			const std::string& associatedObject = ""
		) {
			if(size == 0){
				UT::Ut_Logger::Error(typeid(NoneObj), "Attempted to copy 0 bytes to device buffer {0}", associatedObject);
				return;
			}

			SingleTimeCommand singleTimeCommand = beginSingleTimeCommands(device, commandPool, queue);

			VkBufferCopy copyRegion = {};
			copyRegion.srcOffset = srcOffset; // optional
			copyRegion.dstOffset = dstOffset; // optional
			copyRegion.size = size;
			vkCmdCopyBuffer(singleTimeCommand.commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

			endSingleTimeCommands(device, singleTimeCommand);
		}

        static uint32_t getMemoryTypeIndex(
            PhysicalDevice* activePhysicalDevice,
            uint32_t typeBits, 
            VkMemoryPropertyFlags properties
        ) {
            if(activePhysicalDevice == nullptr){
                UT::Ut_Logger::RuntimeError(typeid(NoneObj), "No active physical device is set. Run setPhysicalDevice first!");
            }

			VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
			vkGetPhysicalDeviceMemoryProperties(activePhysicalDevice->physicalDevice, &deviceMemoryProperties);
			for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++) {
				if ((typeBits & 1) == 1) {
					if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
						return i;
					}
				}
				typeBits >>= 1;
			}
			return 0;
		}

        static VkResult createBuffer(
            PhysicalDevice* activePhysicalDevice,
            VkDevice device,
			VkBufferUsageFlags usageFlags,
			VkMemoryPropertyFlags memoryPropertyFlags,
			VkBuffer& buffer,
			VkDeviceMemory& memory,
			VkDeviceSize size,
			void* data
		) {
			// Create the buffer handle
			VkBufferCreateInfo bufferCreateInfo{};
			bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCreateInfo.usage = usageFlags;
			bufferCreateInfo.size = size;
			bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			Vk_CheckVkResult(typeid(NoneObj), vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer), "Unable to create buffer");

			// Create the memory backing up the buffer handle
			VkMemoryRequirements memReqs;
			VkMemoryAllocateInfo memAllocInfo{};
			memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			vkGetBufferMemoryRequirements(device, buffer, &memReqs);
			memAllocInfo.allocationSize = memReqs.size;

			memAllocInfo.memoryTypeIndex = getMemoryTypeIndex(activePhysicalDevice, memReqs.memoryTypeBits, memoryPropertyFlags);
			VkResult res = vkAllocateMemory(device, &memAllocInfo, nullptr, &memory);
			if (res != VK_SUCCESS) {
				if (res == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
					throw OutOfDeviceMemoryException();
				}
				else {
					UT::Ut_Logger::RuntimeError(typeid(NoneObj), "failed to allocate buffer memory!");
				}
			}

			if (data != nullptr) {
				void* mapped;
				Vk_CheckVkResult(typeid(NoneObj), vkMapMemory(device, memory, 0, size, 0, &mapped), "Unable to map memory for buffer creation");
				memcpy(mapped, data, size);
				vkUnmapMemory(device, memory);
			}

			Vk_CheckVkResult(typeid(NoneObj), vkBindBufferMemory(device, buffer, memory, 0), "Unable to bind buffer memory to device");

			return VK_SUCCESS;
		}

        static void devicePropertyFlagToString(const VkMemoryPropertyFlags flags, std::string& propStrShort, std::string& propStrLong) {
			std::string type = "";
			switch (flags) {
			case 0:  
				type = "HOST LOCAL / SHARED"; 
				break;
			case VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT:
				type = "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT"; 
				break;
			case VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT:
				type = "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT";
				break;
			case VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT:
				type = "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT"; 
				break;
			case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT:
				type = "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT"; 
				break;
			case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT:
				type = "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT"; 
				break;
			case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT:
				type = "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT";
				break;
			case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT:  
				type = "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT"; 
				break;
			case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT:  
				type = "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT"; 
				break;
			case VK_MEMORY_PROPERTY_PROTECTED_BIT:  
				type = "VK_MEMORY_PROPERTY_PROTECTED_BIT"; 
				break;
			case VK_MEMORY_PROPERTY_PROTECTED_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT: 
				type = "VK_MEMORY_PROPERTY_PROTECTED_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT"; 
				break;
			case VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD: 
				type = "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD"; 
				break;
			case VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD: 
				type = "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD"; 
				break;
			case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD: 
				type = "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD"; 
				break;
			case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD: 
				type = "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD"; 
				break;
			case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD: 
				type = "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD"; 
				break;
			case VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD: 
				type = "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD"; 
				break;
			case VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD: 
				type = "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD"; 
				break;
			case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD: 
				type = "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD"; 
				break;
			case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD: 
				type = "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD"; 
				break;
			case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD: 
				type = "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD"; 
				break;
			case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV: 
				type = "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV"; 
				break;
			default: break;
			}

			propStrLong = std::string(type);

			while (Vk_Lib::replace(type, "VK_MEMORY_PROPERTY_", "")) {};
			while (Vk_Lib::replace(type, "_BIT", "")) {};
			while (Vk_Lib::replace(type, "_", " ")) {};

			propStrShort = std::string(type);
		}

    };
}