#pragma once

#include <vector>
#include <unordered_map>

#include "../Defines.h"
#include "../external/tabulate/single_include/tabulate/tabulate.hpp"
#include "../Vk_CI.hpp"
#include "Vk_PhysicalDeviceQueueLib.hpp"

namespace VK5 {
    typedef int TPhysicalDeviceIndex;
    class Vk_PhysicalDevice;
    typedef std::unordered_map<TPhysicalDeviceIndex, Vk_PhysicalDevice> TPhysicalDevices;

    class Vk_PhysicalDeviceLib{
    public:
        struct ExtensionSupport {
            bool swapchain;
            bool memoryBudget;
            bool wideLines;
            bool timelineSemaphore;
        };

        /**
         * This struct groups all the information about a physical device.
         * PR stands for public relations.
         */
        struct PhysicalDevicePR {
            VkPhysicalDeviceProperties properties;
            VkPhysicalDeviceFeatures v10features;
            VkPhysicalDeviceVulkan12Features v12features {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
            VkSampleCountFlagBits maxUsableSampleCount;
            std::vector<VkExtensionProperties> availableExtensions;
            ExtensionSupport extensionSupport;
        };

        /**
         * Query
         *  - VkPhysicalDeviceProperties
         *  - VkPhysicalDeviceFeatures
         *  - VkSampleCountFlagBits
         *  - all supported VkExtensionProperties
         */
        static PhysicalDevicePR queryPhysicalDevicePr(VkPhysicalDevice physicalDevice){
            PhysicalDevicePR pr;

            // query basic physical device stuff like type and supported vulkan version
			vkGetPhysicalDeviceProperties(physicalDevice, &pr.properties);
			// query exotic stuff about the device like viewport rendering or 64 bit floats
            VkPhysicalDeviceFeatures2 features2;
            features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            features2.pNext = &pr.v12features;
            features2.features = pr.v10features;
            vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);
            pr.v10features = features2.features;
            // get the maximum usable sample count
			VkSampleCountFlags counts = std::min(pr.properties.limits.framebufferColorSampleCounts, pr.properties.limits.framebufferDepthSampleCounts);
            // assign useful enum designation to max usable sample count
            pr.maxUsableSampleCount = _getMaxUsableSampleCount(counts);

            // query all available extensions
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
            pr.availableExtensions.resize(extensionCount);
			vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount,pr.availableExtensions.data());

            pr.extensionSupport.swapchain =_querySwapchainExtensionSupport(pr);
            pr.extensionSupport.memoryBudget =_queryMemoryBudgetExtensionSupport(pr);
            pr.extensionSupport.wideLines =_queryWideLinesSupport(pr);
            pr.extensionSupport.timelineSemaphore = _queryTimelineSemaphoreSupport(pr);

            return pr;
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
        static bool _querySwapchainExtensionSupport(const PhysicalDevicePR& pr) {
            for(const auto& extension : pr.availableExtensions) {
                if(strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) return true;
            }
            return false;
        }

        static bool _queryMemoryBudgetExtensionSupport(const PhysicalDevicePR& pr) {
            for(const auto& extension : pr.availableExtensions) {
                if(strcmp(extension.extensionName, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME) == 0) return true;
            }
            return false;
        }

        static bool _queryTimelineSemaphoreSupport(const PhysicalDevicePR& pr) {
            // for(const auto& extension : pr.availableExtensions) {
            //     if(strcmp(extension.extensionName, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME) == 0) return true;
            // }
            // return false;
            return pr.v12features.timelineSemaphore == VK_TRUE;
        }

        static bool _queryWideLinesSupport(const PhysicalDevicePR& pr) {
            return pr.v10features.wideLines == VK_TRUE;
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