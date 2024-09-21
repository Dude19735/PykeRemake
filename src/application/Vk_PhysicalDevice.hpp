#pragma once

#include <unordered_map>

#include "../Defines.h"
#include "Vk_PhysicalDeviceLib.hpp"
#include "Vk_PhysicalDeviceQueue.hpp"

namespace VK5 {
    class Vk_PhysicalDevice{
    private:
        TPhysicalDeviceIndex _index;
        VkPhysicalDevice _physicalDevice;
        Vk_PhysicalDeviceLib::PhysicalDevicePR _pr;
        Vk_PhysicalDeviceQueue _physicalDeviceQueues;
        VkDevice _device;
    public:
        /**
         * Enumerate all available physical devices.
         * Throw a runtime exception if something goes wrong.
         */
        static std::vector<VkPhysicalDevice> enumeratePhysicalDevices(VkInstance vkInstance){
            // see how many vulkan capable devices we can find
            uint32_t deviceCount;
            VkResult res = vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
            Vk_CheckVkResult(typeid(NoneObj), res, "Failed to enumerate physical devices");

            // if we don't find any suitable devices, throw an exception because the program won't work anyways
			if (deviceCount == 0) UT::Ut_Logger::RuntimeError(typeid(NoneObj), "Failed to find devices with Vulkan support!");

            // retrieve the real physical devices into a vector
			std::vector<VkPhysicalDevice> vkPhysicalDevices(deviceCount);
			res = vkEnumeratePhysicalDevices(vkInstance, &deviceCount, vkPhysicalDevices.data());
			if(res == VK_INCOMPLETE) UT::Ut_Logger::RuntimeError(typeid(NoneObj), "Fewer GPUs than actually presend in hardware were detected by the graphics driver. Note that in the newest version of Windows, the GPU is selected per App and all GPUs are visible by default. It can be that the vendor of one of the GPUs hides the respective other GPU. This causes VK_INCOMPLETE to be returned. If you use a dual GPU system, select the GPU that does not cause these problems and select the GPU for the viewer using Vk_DevicePreference.");
			else Vk_CheckVkResult(typeid(NoneObj), res, "Failed to load physical devices");

            return vkPhysicalDevices;
        }

        Vk_PhysicalDevice(
            TPhysicalDeviceIndex index,
            VkPhysicalDevice physicalDevice,
            const Vk_PhysicalDeviceLib::PhysicalDevicePR& pr,
            const std::vector<Vk_GpuOp>& opPriorities
        )
        :
        _index(index),
        _physicalDevice(physicalDevice),
        _pr(pr),
        _physicalDeviceQueues(physicalDevice, opPriorities),
        _device(_allocateLogicalDevice(_physicalDevice, _pr, _physicalDeviceQueues))
        {}

        Vk_PhysicalDevice(Vk_PhysicalDevice&& other)
        :
        _index(other._index),
        _physicalDevice(other._physicalDevice),
        _pr(std::move(other._pr)),
        _physicalDeviceQueues(std::move(other._physicalDeviceQueues)),
        _device(other._device)
        {
            other._device = nullptr;
            other._physicalDevice = nullptr;
        }

        ~Vk_PhysicalDevice(){
            if(_device != nullptr) vkDestroyDevice(_device, nullptr);
        }

        VkPhysicalDevice vk_physicalDevice() const { return _physicalDevice; }
        const Vk_PhysicalDeviceLib::PhysicalDevicePR& physicalDevicePR() const { return _pr; }
        const Vk_PhysicalDeviceQueue& physicalDeviceQueues() const { return _physicalDeviceQueues; }

    private:
        VkDevice _allocateLogicalDevice(VkPhysicalDevice physicalDevice, const Vk_PhysicalDeviceLib::PhysicalDevicePR& pr, const Vk_PhysicalDeviceQueue& physicalDeviceQueues){
            // get map to find all necessary queues (queues with skills that fit some opPriority)
            int largestFamily;
            const auto deviceQueueFamilyMap = Vk_PhysicalDeviceLib::getDeviceQueueFamilyMap(_physicalDeviceQueues.queueFamilies(), largestFamily); 

			// add all available device physicalDeviceQueues to the create info
            auto queueCreateInfos = Vk_PhysicalDeviceLib::getDeviceQueueCreateInfo(deviceQueueFamilyMap, largestFamily);

            // create the device create info with required extensions etc
            auto deviceCreateInfo = Vk_PhysicalDeviceLib::getDeviceCreateInfo(queueCreateInfos.data, pr);            

            VkDevice device;
			Vk_CheckVkResult(typeid(NoneObj), vkCreateDevice(physicalDevice, &deviceCreateInfo.data, nullptr, &device), "Failed to create logical device");

            _physicalDeviceQueues.assign(device, deviceQueueFamilyMap);

            return device;
        }
    };
}