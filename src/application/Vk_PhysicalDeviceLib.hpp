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
        };

        /**
         * This struct groups all the information about a physical device.
         * PR stands for public relations.
         */
        struct PhysicalDevicePR {
            VkPhysicalDeviceProperties properties;
            VkPhysicalDeviceFeatures features;
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

            capabilities.extensionSupport.swapchain =_querySwapchainExtensionSupport(capabilities);
            capabilities.extensionSupport.memoryBudget =_queryMemoryBudgetExtensionSupport(capabilities);
            capabilities.extensionSupport.wideLines =_queryWideLinesSupport(capabilities);

            return capabilities;
        }

        static TDeviceDeviceQueueFamilyMap getDeviceQueueFamilyMap(const TQueueFamilies& queueFamilies, /*out*/int& largestFamily){
            TDeviceDeviceQueueFamilyMap uniqueQueueFamilies;
            largestFamily = 0;
            for(const auto& f : queueFamilies){
                auto& family = f.second;
                if(family.opPriorities.size() == 0) continue;
                if(family.queueCount > largestFamily) largestFamily = family.queueCount;
                uniqueQueueFamilies.insert({family.queueFamilyIndex, UT::Ut_Std::vec_range(0U, family.queueCount)});
            }
            return uniqueQueueFamilies;
        }

        static Vk_CI::VkDeviceQueueCreateInfo_W getDeviceQueueCreateInfo(const TDeviceDeviceQueueFamilyMap& familyMap, int largestFamily){
            Vk_CI::VkDeviceQueueCreateInfo_W w;
            w.queuePriority = std::vector<float>(largestFamily, 1.0f);
			for (const auto& uqf : familyMap) {
				VkDeviceQueueCreateInfo queueCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
				queueCreateInfo.queueFamilyIndex = uqf.first;
				queueCreateInfo.queueCount = static_cast<uint32_t>(uqf.second.size());
				queueCreateInfo.pQueuePriorities = w.queuePriority.data();
				w.data.push_back(queueCreateInfo);
            }

            return w;
        }

        static Vk_CI::VkDeviceCreateInfo_W getDeviceCreateInfo(const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos, const Vk_PhysicalDeviceLib::PhysicalDevicePR& pr){
            // NOTE: this one actually **allocates** the queues too => need to get all of them with vkGetDeviceQueue
            Vk_CI::VkDeviceCreateInfo_W w = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
			w.data.pQueueCreateInfos = queueCreateInfos.data();
			w.data.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
			w.data.pEnabledFeatures = &pr.features;

            if(pr.extensionSupport.memoryBudget) w.deviceExtensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
            if(pr.extensionSupport.swapchain) w.deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

			w.data.enabledExtensionCount = static_cast<uint32_t>(w.deviceExtensions.size());
			w.data.ppEnabledExtensionNames = w.deviceExtensions.data();

            return w;
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