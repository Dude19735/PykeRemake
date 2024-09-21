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
        struct Queue {
            uint32_t familyIndex;
            uint32_t queueIndex;
            VkQueue vkQueue;
        };

        // {PhysicalDeviceIndex : {TQueueFamilyIndex : Vk_QueueFamily}}
        TQueueFamilies _queueFamilies;
        std::unique_ptr<Queue[]> _queues;
        TQueueSize _queuesSize;
        std::unordered_map<Vk_GpuOp, std::vector<Queue*>> _queuesMap;

    public:
        Vk_PhysicalDeviceQueue(VkPhysicalDevice vkPhysicalDevice, const std::vector<Vk_GpuOp>& opPriorities)
        :
        _queueFamilies(Vk_PhysicalDeviceQueueLib::queryQueueFamilies(vkPhysicalDevice, opPriorities)),
        _queues(nullptr),
        _queuesSize(0),
        _queuesMap({})
        {}

        Vk_PhysicalDeviceQueue(Vk_PhysicalDeviceQueue&& other)
        :
        _queueFamilies(std::move(other._queueFamilies)),
        _queues(std::move(other._queues))
        {}

        const TQueueFamilies& queueFamilies() const { return _queueFamilies; }

        void assign(VkDevice device, const TDeviceDeviceQueueFamilyMap& deviceQueueFamilyMap){
            for(const auto& map : deviceQueueFamilyMap) _queuesSize += static_cast<TQueueSize>(map.second.size());
            _queues = std::make_unique<Queue[]>(_queuesSize);

            size_t s=0;
            for(const auto& family : deviceQueueFamilyMap) {
                for(const auto& i : family.second) vkGetDeviceQueue(device, family.first, i, &_queues[s++].vkQueue);
            };

            for(TQueueIndex i=0; i<_queuesSize; ++i){
                auto& queue = _queues[i];
                const auto& ops = _queueFamilies.at(queue.familyIndex).opPriorities;
                for(const auto& op : ops) _queuesMap[op].push_back(&queue);
                std::cout << "lol" << std::endl;
            }
        }
    };
}