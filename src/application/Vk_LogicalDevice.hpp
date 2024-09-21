#pragma once

#include "../Defines.h"
#include "Vk_LogicalDeviceLib.hpp"
#include "Vk_PhysicalDeviceQueue.hpp"

namespace VK5{
    class Vk_LogicalDevice{
    private:
        VkDevice _vkDevice;
    public:
        Vk_LogicalDevice(VkPhysicalDevice physicalDevice, const Vk_PhysicalDeviceLib::PhysicalDevicePR& pr, const Vk_PhysicalDeviceQueue& physicalDeviceQueues)
        :
        _vkDevice(_createLogicalDevice(physicalDevice, pr, physicalDeviceQueues))
        {}

        Vk_LogicalDevice(Vk_LogicalDevice&& other)
        :
        _vkDevice(other._vkDevice)
        {
            other._vkDevice = nullptr;
        }

        ~Vk_LogicalDevice(){
            if(_vkDevice != nullptr) vkDestroyDevice(_vkDevice, nullptr);
        }

        VkDevice vk_device() const { return _vkDevice; }

    private:
        VkDevice _createLogicalDevice(VkPhysicalDevice physicalDevice, const Vk_PhysicalDeviceLib::PhysicalDevicePR& pr, const Vk_PhysicalDeviceQueue& physicalDeviceQueues){
			// add all available device physicalDeviceQueues to the create info
            auto queueCreateInfos = Vk_LogicalDeviceLib::getDeviceQueueCreateInfo(physicalDeviceQueues.queueFamilyMap(), physicalDeviceQueues.largestQueueFamilySize());

            // create the device create info with required extensions etc
            auto deviceCreateInfo = Vk_LogicalDeviceLib::getDeviceCreateInfo(queueCreateInfos.data, pr);            

            VkDevice device;
			Vk_CheckVkResult(typeid(NoneObj), vkCreateDevice(physicalDevice, &deviceCreateInfo.data, nullptr, &device), "Failed to create logical device");

            return device;
        }
    };
}