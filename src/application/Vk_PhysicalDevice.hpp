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
        Vk_PhysicalDeviceExtensionsSupport _extensionsSupport;

    public:
        Vk_PhysicalDeviceQueue Queues;

        Vk_PhysicalDevice(
            TPhysicalDeviceIndex index,
            VkPhysicalDevice physicalDevice,
            const Vk_PhysicalDeviceLib::PhysicalDevicePR& pr, 
            const Vk_PhysicalDeviceExtensionsSupport& extensionsSupport,
            const std::vector<Vk_GpuOp>& opPriorities
        )
        :
        _index(index),
        _physicalDevice(physicalDevice),
        _pr(pr),
        _extensionsSupport(extensionsSupport),
        Queues(physicalDevice, opPriorities)
        {}

        VkPhysicalDevice vk_physicalDevice() const { return _physicalDevice; }
        const Vk_PhysicalDeviceLib::PhysicalDevicePR& physicalDevicePR() const { return _pr; }
    };
}