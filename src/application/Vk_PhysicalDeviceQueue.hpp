#pragma once

#include <unordered_map>
#include <vector>
#include <sstream>
#include <set>
#include <algorithm>

#include "../Defines.h"
#include "Vk_PhysicalDeviceQueueLib.hpp"
#include "Vk_PhysicalDeviceLib.hpp"
#include "Vk_Surface.hpp"

namespace VK5 {
    class Vk_PhysicalDeviceQueue {
    private:
        // {PhysicalDeviceIndex : {TQueueFamilyIndex : Vk_QueueFamily}}
        TQueueFamilies _queueFamilies;
    public:

        Vk_PhysicalDeviceQueue(VkPhysicalDevice vkPhysicalDevice, const std::vector<Vk_GpuOp>& opPriorities)
        :
        _queueFamilies(Vk_PhysicalDeviceQueueLib::queryQueueFamilies(vkPhysicalDevice, opPriorities))
        {}

        const TQueueFamilies& queueFamilies() const { return _queueFamilies; }

    private:
    };
}