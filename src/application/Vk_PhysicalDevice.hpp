#pragma once

#include <unordered_map>

#include "../Defines.h"
#include "Vk_PhysicalDeviceLib.hpp"
#include "Vk_PhysicalDeviceQueue.hpp"
#include "Vk_LogicalDevice.hpp"
#include "Vk_LogicalDeviceQueue.hpp"

namespace VK5 {
    class Vk_PhysicalDevice{
    private:
        TPhysicalDeviceIndex _index;
        VkPhysicalDevice _physicalDevice;
        Vk_PhysicalDeviceLib::PhysicalDevicePR _pr;
        Vk_PhysicalDeviceQueue _physicalDeviceQueues;
        Vk_LogicalDevice _logicalDevice;
        Vk_LogicalDeviceQueue _logicalDeviceQueues;
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
        _logicalDevice(_physicalDevice, _pr, _physicalDeviceQueues),
        _logicalDeviceQueues(_logicalDevice.vk_device(), _physicalDeviceQueues)
        {}

        Vk_PhysicalDevice(Vk_PhysicalDevice&& other)
        :
        _index(other._index),
        _physicalDevice(other._physicalDevice),
        _pr(std::move(other._pr)),
        _physicalDeviceQueues(std::move(other._physicalDeviceQueues)),
        _logicalDevice(std::move(other._logicalDevice)),
        _logicalDeviceQueues(std::move(other._logicalDeviceQueues))
        {
            other._physicalDevice = nullptr;
        }

        ~Vk_PhysicalDevice(){}

        VkPhysicalDevice vk_physicalDevice() const { return _physicalDevice; }
        const Vk_PhysicalDeviceLib::PhysicalDevicePR& physicalDevicePR() const { return _pr; }
        const Vk_PhysicalDeviceQueue& physicalDeviceQueues() const { return _physicalDeviceQueues; }
        const Vk_LogicalDeviceQueue& logicalDeviceQueues() const { return _logicalDeviceQueues; }
    };
}