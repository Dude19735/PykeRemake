#pragma once

#include "../Defines.h"
#include "../Vk_CI.hpp"
#include "Vk_PhysicalDeviceLib.hpp"

namespace VK5{
    class Vk_LogicalDeviceLib{
    public:
        static Vk_CI::VkDeviceQueueCreateInfo_W getDeviceQueueCreateInfo(const TDeviceQueueFamilyMap& familyMap, int largestFamily){
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
			w.data.pEnabledFeatures = &pr.v10features;
            w.data.pNext = &pr.v12features;

            if(pr.extensionSupport.memoryBudget) w.deviceExtensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
            if(pr.extensionSupport.swapchain) w.deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
            if(pr.extensionSupport.timelineSemaphore) w.deviceExtensions.push_back(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

			w.data.enabledExtensionCount = static_cast<uint32_t>(w.deviceExtensions.size());
			w.data.ppEnabledExtensionNames = w.deviceExtensions.data();

            return w;
        }
    };
}