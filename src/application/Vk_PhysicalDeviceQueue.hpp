#pragma once

#include <unordered_map>
#include <map>
#include <queue>
#include <vector>
#include <sstream>
#include <set>
#include <algorithm>
#include <shared_mutex>

#include "../Defines.h"
#include "Vk_PhysicalDeviceQueueLib.hpp"
#include "Vk_PhysicalDeviceLib.hpp"

namespace VK5 {
    class Vk_PhysicalDeviceQueue {
    private:
        TQueueFamilies _queueFamilies;
        TQueueSize _largestFamilySize;
        TDeviceQueueFamilyMap _deviceQueueFamilyMap;
        std::set<Vk_GpuOp> _supportedOpTypes;

    public:
        Vk_PhysicalDeviceQueue(VkPhysicalDevice vkPhysicalDevice, const std::vector<Vk_GpuOp>& opPriorities)
        :
        _queueFamilies(Vk_PhysicalDeviceQueueLib::queryQueueFamilies(vkPhysicalDevice, opPriorities)),
        _deviceQueueFamilyMap(Vk_PhysicalDeviceQueueLib::getDeviceQueueFamilyMap(_queueFamilies, _largestFamilySize)),
        _supportedOpTypes(Vk_PhysicalDeviceQueueLib::getSupportedOpTypes(_queueFamilies))
        {}

        Vk_PhysicalDeviceQueue(Vk_PhysicalDeviceQueue&& other)
        :
        _queueFamilies(std::move(other._queueFamilies)),
        _supportedOpTypes(std::move(other._supportedOpTypes))
        {}

        ~Vk_PhysicalDeviceQueue(){}

        const TQueueFamilies& queueFamilies() const { return _queueFamilies; }
        const TQueueSize largestQueueFamilySize() const { return _largestFamilySize; }
        const TDeviceQueueFamilyMap& queueFamilyMap() const { return _deviceQueueFamilyMap; }
    };
}