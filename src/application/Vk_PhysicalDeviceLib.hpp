#pragma once

#include <vector>
#include <unordered_map>

#include "../Defines.h"
#include "../external/tabulate/single_include/tabulate/tabulate.hpp"
#include "Vk_PhysicalDeviceQueueLib.hpp"
#include "Vk_Instance.hpp"

namespace VK5 {
    typedef int TPhysicalDeviceIndex;
    class Vk_PhysicalDevice;
    typedef std::unordered_map<TPhysicalDeviceIndex, Vk_PhysicalDevice> TPhysicalDevices;

    /**
     * This struct contains a bunch of bools to make support test easier and more explicit
     */
    struct Vk_PhysicalDeviceExtensionsSupport {
        bool swapchain;
        bool memoryBudget;
        bool wideLines;
    };

    class Vk_PhysicalDeviceLib{
    public:
        /**
         * This struct groups all the information about a physical device.
         * PR stands for public relations.
         */
        struct PhysicalDevicePR {
            VkPhysicalDeviceProperties properties;
            VkPhysicalDeviceFeatures features;
            VkSampleCountFlagBits maxUsableSampleCount;
            std::vector<VkExtensionProperties> availableExtensions;
        };

        /**
         * Enumerate all available physical devices.
         * Throw a runtime exception if something goes wrong.
         */
        static std::vector<VkPhysicalDevice> enumeratePhysicalDevices(const Vk_Instance& instance){
            // see how many vulkan capable devices we can find
            uint32_t deviceCount;
            VkResult res = vkEnumeratePhysicalDevices(instance.vk_instance(), &deviceCount, nullptr);
            Vk_CheckVkResult(typeid(NoneObj), res, "Failed to enumerate physical devices");

            // if we don't find any suitable devices, throw an exception because the program won't work anyways
			if (deviceCount == 0) UT::Ut_Logger::RuntimeError(typeid(NoneObj), "Failed to find devices with Vulkan support!");

            // retrieve the real physical devices into a vector
			std::vector<VkPhysicalDevice> vkPhysicalDevices(deviceCount);
			res = vkEnumeratePhysicalDevices(instance.vk_instance(), &deviceCount, vkPhysicalDevices.data());
			if(res == VK_INCOMPLETE) UT::Ut_Logger::RuntimeError(typeid(NoneObj), "Fewer GPUs than actually presend in hardware were detected by the graphics driver. Note that in the newest version of Windows, the GPU is selected per App and all GPUs are visible by default. It can be that the vendor of one of the GPUs hides the respective other GPU. This causes VK_INCOMPLETE to be returned. If you use a dual GPU system, select the GPU that does not cause these problems and select the GPU for the viewer using Vk_DevicePreference.");
			else Vk_CheckVkResult(typeid(NoneObj), res, "Failed to load physical devices");

            return vkPhysicalDevices;
        }

        /**
         * Query
         *  - VkPhysicalDeviceProperties
         *  - VkPhysicalDeviceFeatures
         *  - VkSampleCountFlagBits
         *  - all supported VkExtensionProperties
         */
        static PhysicalDevicePR queryPhysicalDeviceCapabilities(VkPhysicalDevice physicalDevice){
            PhysicalDevicePR capabilities;

            // query basic physical device stuff like type and supported vulkan version
			vkGetPhysicalDeviceProperties(physicalDevice, &capabilities.properties);
			// query exotic stuff about the device like viewport rendering or 64 bit floats
			vkGetPhysicalDeviceFeatures(physicalDevice, &capabilities.features);
            // get the maximum usable sample count
			VkSampleCountFlags counts = std::min(capabilities.properties.limits.framebufferColorSampleCounts, capabilities.properties.limits.framebufferDepthSampleCounts);
            // assign useful enum designation to max usable sample count
            capabilities.maxUsableSampleCount = _getMaxUsableSampleCount(counts);

            // query all available extensions
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
            capabilities.availableExtensions.resize(extensionCount);
			vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount,capabilities.availableExtensions.data());

            return capabilities;
        }

        /**
         * Check for explicit support of capabilities using a Vk_PhysicalDeviceCapabilities.
         * Returns a struct with a bunch of bools.
         * Can be extended with all kinds of support requirements.
         */
        static Vk_PhysicalDeviceExtensionsSupport queryPhysicalDeviceExtensionsSupport(const PhysicalDevicePR& capabilities){
            return Vk_PhysicalDeviceExtensionsSupport {
                .swapchain=_querySwapchainExtensionSupport(capabilities),
                .memoryBudget=_queryMemoryBudgetExtensionSupport(capabilities),
                .wideLines=_queryWideLinesSupport(capabilities)
            };
        }

        static void tableStream(
            const std::unordered_map<TPhysicalDeviceIndex, PhysicalDevicePR>& physicalDevices,
            const std::unordered_map<TPhysicalDeviceIndex, TQueueFamilies>& queueFamilies, 
            /*out*/std::ostream& stream
        ){
			tabulate::Table table;
            table.add_row({"GPU", "DeviceIndex", "QueueIndex", "VkQueueFamilyProperties", "OpPriority", "#Queues", "Min Transf [w,h,d]", "Present Capable"});
            table.format().font_style({ tabulate::FontStyle::bold });
			table[0].format()
				.padding_top(1)
				.padding_bottom(1)
				.font_align(tabulate::FontAlign::center)
				.font_style({ tabulate::FontStyle::underline })
				.font_background_color(tabulate::Color::red);

			for(const auto& dev : physicalDevices){
				std::string physicalDeviceName = dev.second.properties.deviceName;
				std::string physicalDeviceIndex = std::to_string(dev.first);
				for(const auto& fp : queueFamilies.at(dev.first)){
					const auto& family = fp.second;

					std::string famMinGranW = std::to_string(family.minImageTransferGranularity.width);
					std::string famMinGranH = std::to_string(family.minImageTransferGranularity.height);
					std::string famMinGranD = std::to_string(family.minImageTransferGranularity.depth);

					table.add_row({
						physicalDeviceName,
						physicalDeviceIndex,
						std::to_string(family.queueFamilyIndex),
						Vk_PhysicalDeviceQueueLib::queueFamilyFlagBitsSet2Str(family.flagBits),
                        Vk_PhysicalDeviceQueueLib::queueFamilyOpPriorityVec2Str(family.opPriorities),
						std::to_string(family.queueCount),
						std::string("[") + famMinGranW + ","  + famMinGranH + "," + famMinGranD + "]",
						Vk_PhysicalDeviceQueueLib::queueFamilyPresentCapable2Str(family.presentCapable)
					});
				}
			}

            table.column(0).format().font_color(tabulate::Color::red);
            table.column(1).format().font_color(tabulate::Color::red);
            table.column(2).format().font_color(tabulate::Color::blue);
			table.column(3).format().font_color(tabulate::Color::cyan);
            table.column(4).format().font_color(tabulate::Color::red);
            table.column(5).format().font_color(tabulate::Color::cyan);
            table.column(6).format().font_color(tabulate::Color::cyan);
            table.column(7).format().font_color(tabulate::Color::cyan);

            table[0][0].format().font_background_color(tabulate::Color::red).font_color(tabulate::Color::white);
            table[0][1].format().font_background_color(tabulate::Color::red).font_color(tabulate::Color::white);
            table[0][2].format().font_background_color(tabulate::Color::blue).font_color(tabulate::Color::white);
			table[0][3].format().font_background_color(tabulate::Color::cyan).font_color(tabulate::Color::blue);
            table[0][4].format().font_background_color(tabulate::Color::red).font_color(tabulate::Color::white);
            table[0][5].format().font_background_color(tabulate::Color::cyan).font_color(tabulate::Color::blue);
            table[0][6].format().font_background_color(tabulate::Color::cyan).font_color(tabulate::Color::blue);
            table[0][7].format().font_background_color(tabulate::Color::cyan).font_color(tabulate::Color::blue);

            stream << table << std::endl;
        }

    private:
        static bool _querySwapchainExtensionSupport(const PhysicalDevicePR& capabilities) {
            for(const auto& extension : capabilities.availableExtensions) {
                if(strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) return true;
            }
            return false;
        }

        static bool _queryMemoryBudgetExtensionSupport(const PhysicalDevicePR& capabilities) {
            for(const auto& extension : capabilities.availableExtensions) {
                if(strcmp(extension.extensionName, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME) == 0) return true;
            }
            return false;
        }

        static bool _queryWideLinesSupport(const PhysicalDevicePR& capabilities) {
            return capabilities.features.wideLines == VK_TRUE;
        }

        static VkSampleCountFlagBits _getMaxUsableSampleCount(VkSampleCountFlags counts){
            if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
            if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
            if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
            if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
            if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
            if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;
            return VK_SAMPLE_COUNT_1_BIT;
        }
    };
}